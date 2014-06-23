#include <stdio.h>
#include <stdlib.h>
#include "onviflib.h"

int main(int argc, char **argv)
{
  errStatus status = ERR_OK;
  char deviceAddress[50];
  t_videoResolution newResolution;
  
  status = discoverDevices(deviceAddress);
  if(status != ERR_OK)
    goto exitLabel;
  printf("Received probe matches %s\n", deviceAddress);

  status = connectCamera(deviceAddress);
  if(status != ERR_OK)
    goto exitLabel;
  //printf("Media URL set to %s\n", mediaEP);

  status = getSupportedResolutions();
  if(status != ERR_OK)
    goto exitLabel;

  
  newResolution.Encoding = tt__VideoEncoding__H264;
  newResolution.width = 1280;
  newResolution.height = 720;
  status = setResolution(newResolution);
  if(status == ERR_RESOLUTION_NOT_SUPPORTED)
    printf("Resolution not supported\n");
  else
    printf("Resolution supported\n");
  if(status != ERR_OK)
    goto exitLabel;

#if 0
   struct _trt__GetVideoSourceConfiguration getVideoSourceConfigurationRequest;
   getVideoSourceConfigurationRequest.ConfigurationToken = (char *)malloc(sizeof(videoSrcToken));
   strcpy(getVideoSourceConfigurationRequest.ConfigurationToken, videoSrcToken);
   struct _trt__GetVideoSourceConfigurationResponse getVideoSourceConfigurationResponse;
   soap_call___trt__GetVideoSourceConfiguration(&soap, mediaEP, NULL,  &getVideoSourceConfigurationRequest, &getVideoSourceConfigurationResponse);
   if(getVideoSourceConfigurationResponse.Configuration == NULL)
     {
       printf("No video src config for %s\n", videoSrcToken);
       status = 1;
       goto exit;
     }
     else
     {
       printf("Video config available\n");
     }

   struct _trt__SetVideoSourceConfiguration setVideoSourceConfigurationRequest;
   getVideoSourceConfigurationResponse.Configuration->Bounds->width = 1280;//640;
   getVideoSourceConfigurationResponse.Configuration->Bounds->height = 720;//360;
   getVideoSourceConfigurationResponse.Configuration->Bounds->x = 0;
   getVideoSourceConfigurationResponse.Configuration->Bounds->y = 0;
   setVideoSourceConfigurationRequest.Configuration = getVideoSourceConfigurationResponse.Configuration;
   struct _trt__SetVideoSourceConfigurationResponse setVideoSourceConfigurationResponse;
   soap_call___trt__SetVideoSourceConfiguration(&soap, mediaEP, NULL, &setVideoSourceConfigurationRequest, &setVideoSourceConfigurationResponse);
#endif
   
 exitLabel:
   cleanup();
   return status;
}
