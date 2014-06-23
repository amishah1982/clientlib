#include "onviflib.h"
#include "wsdd.nsmap"


char mediaEP[50];
char videoEncoderToken[50];
char profileToken[50];


t_videoResolution *supportedResolutions;
int noOfSupportedResolutions = 0;

errStatus cleanup()
{   
  errStatus status = ERR_OK;
  struct soap soap;

  soap_destroy(&soap); // cleanup
  soap_end(&soap); // cleanup
  soap_done(&soap); // close connection (should not use soap struct after this)

 exitLabel:
  return status;
}

errStatus discoverDevices(char *deviceAddress)//, int timeout)//int *noOfDevices, char **ipAddressOfDevices)
{   
  errStatus status = ERR_OK;
  struct soap soap;

  // init soap for UDP 
  soap_init1(&soap, SOAP_IO_UDP);
  // reuse address
  soap.bind_flags = SO_REUSEADDR;
  // bind 
  if (!soap_valid_socket(soap_bind(&soap, NULL, 0, 100)))
  { 
    soap_print_fault(&soap, stderr);
    printf("Error in soap bind\n");
    status = ERR_FAIL;
    goto exitLabel;
  }

  soap.send_timeout = 1; // 1s timeout
  soap.recv_timeout = 1; // 1s timeout

  soap_set_namespaces(&soap, namespaces);

  struct SOAP_ENV__Header header; // the SOAP Header
  soap_default_SOAP_ENV__Header(&soap, &header);
  header.wsa5__MessageID = "uuid:08cb8fdb-150e-407c-8aa6-af648f925fb3"; //need to use uuid code
  header.wsa5__To = "urn:schemas-xmlsoap-org:ws:2005:04:discovery";
  header.wsa5__Action = "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe";
  soap.header = &header;

  struct wsdd__ProbeType getProbeRequest;
  getProbeRequest.Types = "dn:NetworkVideoTransmitter";
  // Send probe message over UDP for device discovery
  if ((soap_send___wsdd__Probe(&soap, "soap.udp://239.255.255.250:3702", NULL, &getProbeRequest)) != SOAP_OK)
    {
      printf("Error sending probe\n");
      soap_print_fault(&soap, stderr);
      status = ERR_FAIL;
      goto exitLabel;
    }

  struct __wsdd__ProbeMatches getProbeMatchesResponse;
  while(1) //need to check for timeout here
    {
      soap_recv___wsdd__ProbeMatches(&soap, &getProbeMatchesResponse);
      if(getProbeMatchesResponse.wsdd__ProbeMatches == NULL)   
	{
	  printf("Nothing received\n");
	}
      else
	{
	  strcpy(deviceAddress, getProbeMatchesResponse.wsdd__ProbeMatches->ProbeMatch->XAddrs);
	  break;
	}
    }

 exitLabel:
  return status;
}

//Connect camera
errStatus connectCamera(char *deviceAddress)
{
  errStatus status = ERR_OK;
  struct soap soap;

  struct _tds__GetCapabilities getCapabilitiesRequest;
  struct _tds__GetCapabilitiesResponse getCapabilitiesResponse;
  soap_call___tds__GetCapabilities(&soap, deviceAddress, NULL, &getCapabilitiesRequest, &getCapabilitiesResponse);
  if(getCapabilitiesResponse.Capabilities == NULL)
    {
      printf("Response NULL\n");
      status = ERR_FAIL;
      goto exitLabel;
    }
  else
    {
      strcpy(mediaEP, getCapabilitiesResponse.Capabilities->Media->XAddr);
      //printf("Response received - Media endpoint %s\n", mediaEP); 
    }
  
 exitLabel:
  return status;
}

errStatus getSupportedResolutions()
{
  errStatus status = ERR_OK;
  struct soap soap;
  
  /*get video encoder configuration options*/
  char profileToken[50];

  struct _trt__GetProfiles getProfilesRequest;
  struct _trt__GetProfilesResponse getProfilesResponse;
  soap_call___trt__GetProfiles(&soap, mediaEP, NULL, &getProfilesRequest, &getProfilesResponse);
  if(getProfilesResponse.Profiles == NULL)
    {
      printf("Get Profiles no response received\n");
      status = ERR_FAIL;
      goto exitLabel;
    }
  else
    {
      //printf("Get Profiles response received\n");
      printf("Current video encoder config [%d %d %d %d]\n Video encoder token %s\n Video source token %s\nProfiles token %s\n", getProfilesResponse.Profiles->VideoEncoderConfiguration->Encoding,
	     getProfilesResponse.Profiles->VideoEncoderConfiguration->RateControl->FrameRateLimit,
	     getProfilesResponse.Profiles->VideoEncoderConfiguration->Resolution->Width,
	     getProfilesResponse.Profiles->VideoEncoderConfiguration->Resolution->Height,
	     getProfilesResponse.Profiles->VideoEncoderConfiguration->token,
	     getProfilesResponse.Profiles->VideoSourceConfiguration->token, 
	     getProfilesResponse.Profiles->token);
      strcpy(videoEncoderToken, getProfilesResponse.Profiles->VideoEncoderConfiguration->token);
      strcpy(profileToken, getProfilesResponse.Profiles->token);
    }
  
  struct _trt__GetVideoEncoderConfigurationOptions getVideoEncoderConfigurationOptionsRequest;
  getVideoEncoderConfigurationOptionsRequest.ConfigurationToken = (char *)malloc(sizeof(videoEncoderToken));
  strcpy(getVideoEncoderConfigurationOptionsRequest.ConfigurationToken, videoEncoderToken);
  getVideoEncoderConfigurationOptionsRequest.ProfileToken = (char *)malloc(sizeof(profileToken));
  strcpy(getVideoEncoderConfigurationOptionsRequest.ProfileToken, profileToken);
  struct _trt__GetVideoEncoderConfigurationOptionsResponse getVideoEncoderConfigurationOptionsResponse;
  soap_call___trt__GetVideoEncoderConfigurationOptions(&soap, mediaEP, NULL,  &getVideoEncoderConfigurationOptionsRequest, &getVideoEncoderConfigurationOptionsResponse);
  if(getVideoEncoderConfigurationOptionsResponse.Options == NULL)
    {
      printf("No options available for video config\n");
      status = ERR_FAIL;
      goto exitLabel;
    }
  else
    {
      if(getVideoEncoderConfigurationOptionsResponse.Options->JPEG != NULL)
	{
	  noOfSupportedResolutions += getVideoEncoderConfigurationOptionsResponse.Options->JPEG->__sizeResolutionsAvailable;
	}	
      if(getVideoEncoderConfigurationOptionsResponse.Options->MPEG4 != NULL)
	{
	  noOfSupportedResolutions += getVideoEncoderConfigurationOptionsResponse.Options->MPEG4->__sizeResolutionsAvailable;
	}	
      if(getVideoEncoderConfigurationOptionsResponse.Options->H264 != NULL)
	{
	  noOfSupportedResolutions += getVideoEncoderConfigurationOptionsResponse.Options->H264->__sizeResolutionsAvailable;
	}	
      printf("No of resolutions availabale %d\n", noOfSupportedResolutions);
      supportedResolutions = (t_videoResolution *)malloc(sizeof(t_videoResolution) * noOfSupportedResolutions);

      int resAvail = 0;
      if(getVideoEncoderConfigurationOptionsResponse.Options->JPEG != NULL)
	{
	  int currAvail = 0;
	  for(currAvail = 0; currAvail < getVideoEncoderConfigurationOptionsResponse.Options->JPEG->__sizeResolutionsAvailable; resAvail++, currAvail++)
	    {
	      supportedResolutions[resAvail].Encoding = tt__VideoEncoding__JPEG;
	      supportedResolutions[resAvail].width = getVideoEncoderConfigurationOptionsResponse.Options->JPEG->ResolutionsAvailable[currAvail].Width;
	      supportedResolutions[resAvail].height = getVideoEncoderConfigurationOptionsResponse.Options->JPEG->ResolutionsAvailable[currAvail].Height;
	      printf("\t[%d %d %d]\n", supportedResolutions[resAvail].Encoding, supportedResolutions[resAvail].width, supportedResolutions[resAvail].height);
	    }
	}	
      if(getVideoEncoderConfigurationOptionsResponse.Options->MPEG4 != NULL)
	{
	  int currAvail = 0;
	  for(currAvail = 0; currAvail < getVideoEncoderConfigurationOptionsResponse.Options->MPEG4->__sizeResolutionsAvailable; resAvail++, currAvail++)
	    {
	      supportedResolutions[resAvail].Encoding = tt__VideoEncoding__MPEG4;
	      supportedResolutions[resAvail].width = getVideoEncoderConfigurationOptionsResponse.Options->MPEG4->ResolutionsAvailable[currAvail].Width;
	      supportedResolutions[resAvail].height = getVideoEncoderConfigurationOptionsResponse.Options->MPEG4->ResolutionsAvailable[currAvail].Height;
	      printf("\t[%d %d %d]\n", supportedResolutions[resAvail].Encoding, supportedResolutions[resAvail].width, supportedResolutions[resAvail].height);
	    }
	}	
      if(getVideoEncoderConfigurationOptionsResponse.Options->H264 != NULL)
	{
	  int currAvail = 0;
	  for(currAvail = 0; currAvail < getVideoEncoderConfigurationOptionsResponse.Options->H264->__sizeResolutionsAvailable; resAvail++, currAvail++)
	    {
	      supportedResolutions[resAvail].Encoding = tt__VideoEncoding__H264;
	      supportedResolutions[resAvail].width = getVideoEncoderConfigurationOptionsResponse.Options->H264->ResolutionsAvailable[currAvail].Width;
	      supportedResolutions[resAvail].height = getVideoEncoderConfigurationOptionsResponse.Options->H264->ResolutionsAvailable[currAvail].Height;
	      printf("\t[%d %d %d]\n", supportedResolutions[resAvail].Encoding, supportedResolutions[resAvail].width, supportedResolutions[resAvail].height);
	    }
	}	
    }
 exitLabel:
  return status;
}

errStatus checkSupportForNewResolution(t_videoResolution newResolution)
{
  errStatus status = ERR_OK;

  int resAvail = 0;
  for(resAvail = 0; resAvail < noOfSupportedResolutions; resAvail++)
    {
      if(!memcmp(&supportedResolutions[resAvail], &newResolution, sizeof(t_videoResolution)))
	 return ERR_OK;
    }

  return ERR_FAIL;    
}

errStatus setResolution(t_videoResolution newResolution)
{
  errStatus status = ERR_OK;
  struct soap soap;
  
  if(checkSupportForNewResolution(newResolution) != ERR_OK)
    {
      status = ERR_RESOLUTION_NOT_SUPPORTED;
      goto exitLabel;
    }

  struct _trt__GetVideoEncoderConfiguration getVideoEncoderConfigurationRequest;
  getVideoEncoderConfigurationRequest.ConfigurationToken = (char *)malloc(sizeof(videoEncoderToken));
  strcpy(getVideoEncoderConfigurationRequest.ConfigurationToken, videoEncoderToken);
  struct _trt__GetVideoEncoderConfigurationResponse getVideoEncoderConfigurationResponse;
  soap_call___trt__GetVideoEncoderConfiguration(&soap, mediaEP, NULL,  &getVideoEncoderConfigurationRequest, &getVideoEncoderConfigurationResponse);
  if(getVideoEncoderConfigurationResponse.Configuration == NULL)
    {
      printf("No video config for %s\n", videoEncoderToken);
      status = ERR_FAIL;
      goto exitLabel;
    }
  else
    {
      printf("Video config available resolution [%d %d] ratecontrol [%d %d]\n",
	     getVideoEncoderConfigurationResponse.Configuration->Resolution->Width,
	     getVideoEncoderConfigurationResponse.Configuration->Resolution->Height,
	     getVideoEncoderConfigurationResponse.Configuration->RateControl->FrameRateLimit,
	     getVideoEncoderConfigurationResponse.Configuration->RateControl->BitrateLimit);
    }

   struct _trt__SetVideoEncoderConfiguration setVideoEncoderConfigurationRequest;
   getVideoEncoderConfigurationResponse.Configuration->Encoding = newResolution.Encoding;
   getVideoEncoderConfigurationResponse.Configuration->Resolution->Width = newResolution.width;
   getVideoEncoderConfigurationResponse.Configuration->Resolution->Height = newResolution.height;
   //getVideoEncoderConfigurationResponse.Configuration->RateControl->FrameRateLimit = 30;//15;
   //getVideoEncoderConfigurationResponse.Configuration->RateControl->BitrateLimit = 4096;//512;
   setVideoEncoderConfigurationRequest.Configuration = getVideoEncoderConfigurationResponse.Configuration;
   struct _trt__SetVideoEncoderConfigurationResponse setVideoEncoderConfigurationResponse;
   soap_call___trt__SetVideoEncoderConfiguration(&soap, mediaEP, NULL, &setVideoEncoderConfigurationRequest, &setVideoEncoderConfigurationResponse);

  struct _trt__GetProfiles getProfilesRequest;
  struct _trt__GetProfilesResponse getProfilesResponse;
  soap_call___trt__GetProfiles(&soap, mediaEP, NULL, &getProfilesRequest, &getProfilesResponse);
  if(getProfilesResponse.Profiles == NULL)
    {
      printf("Get Profiles no response received\n");
      status = ERR_FAIL;
      goto exitLabel;
    }
  else
    {
      printf("Current video encoder config [%d %d %d %d]\n Video encoder token %s\n Video source token %s\nProfiles token %s\n", getProfilesResponse.Profiles->VideoEncoderConfiguration->Encoding,
	     getProfilesResponse.Profiles->VideoEncoderConfiguration->RateControl->FrameRateLimit,
	     getProfilesResponse.Profiles->VideoEncoderConfiguration->Resolution->Width,
	     getProfilesResponse.Profiles->VideoEncoderConfiguration->Resolution->Height,
	     getProfilesResponse.Profiles->VideoEncoderConfiguration->token,
	     getProfilesResponse.Profiles->VideoSourceConfiguration->token, 
	     getProfilesResponse.Profiles->token);
    }

 exitLabel:
  return status;
}

