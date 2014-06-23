#include <stdio.h>
#include <stdlib.h>
#include "soapH.h"
#include "soapStub.h"


#define ERR_OK   0
#define ERR_FAIL 1
#define ERR_RESOLUTION_NOT_SUPPORTED 2

typedef int errStatus;

typedef struct videoResolution
{
  enum tt__VideoEncoding Encoding;
  int width;
  int height;
}t_videoResolution;

errStatus cleanup();
errStatus discoverDevices(char *deviceAddress);
errStatus connectCamera(char *deviceAddress);
errStatus getSupportedResolutions();
errStatus setResolution(t_videoResolution newResolution);

