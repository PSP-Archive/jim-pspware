/*********************************************************************
 * 
 *  Main file for modplayer sample for PSP
 *  adresd 2005
 */
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspiofilemgr.h>
#include <pspctrl.h>
#include <pspaudio.h>
#include <pspdisplay.h>
#include <stdlib.h>
#include <string.h>
#include <pspmoduleinfo.h>
#include <pspaudiolib.h>
#include <pspusb.h>
#include <pspusbstor.h>
#include <pspmodulemgr.h>
#include <pspsdk.h>


#include "codec.h"

//  These are the headers for the different codecs
//  auto-generated by the makefile
#include "codecincs.h"

/* Define the module info section */
PSP_MODULE_INFO("PSPMC", 0x1000, 0, 1);
PSP_MAIN_THREAD_ATTR(0);

/* Define printf, just to make typing easier */
#define printf  pspDebugScreenPrintf

codecStubs stubs[100];
codecStubs *decoder;
unsigned char banner[] = "PSP Media Center v0.90 by John_K & adresd\0";
int errno, __errno;
int codecnum = 0;

/* Example custom exception handler */
void MyExceptionHandler(PspDebugRegBlock * regs)
{
    /* I always felt BSODs were more interesting that white on black */
    pspDebugScreenSetBackColor(0x00FF0000);
    pspDebugScreenSetTextColor(0xFFFFFFFF);
    //pspDebugScreenClear();

    pspDebugScreenSetXY(0,17);
    pspDebugScreenPrintf("\nI regret to inform you your psp has just crashed\n");
    pspDebugScreenPrintf("Exception Details:\n");
    pspDebugDumpException(regs);
    pspDebugScreenSetBackColor(0x00000000);
}


#ifdef USB_ENABLED
//helper function to make things easier
int LoadStartModule(char *path)
{
    u32 loadResult;
    u32 startResult;
    int status;

    loadResult = sceKernelLoadModule(path, 0, NULL);
    if (loadResult & 0x80000000)
	return -1;
    else
	startResult =
	    sceKernelStartModule(loadResult, 0, NULL, &status, NULL);

    if (loadResult != startResult)
	return -2;

    return 0;
}
void usb_init(void)
{
  int retVal;
  //start necessary drivers
  LoadStartModule("flash0:/kd/semawm.prx");
  LoadStartModule("flash0:/kd/usbstor.prx");
  LoadStartModule("flash0:/kd/usbstormgr.prx");
  LoadStartModule("flash0:/kd/usbstorms.prx");
  LoadStartModule("flash0:/kd/usbstorboot.prx");

  //setup USB drivers
  retVal = sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0);
  if (retVal != 0) {
  	printf("Error starting USB Bus driver (0x%08X)\n", retVal);
	  sceKernelSleepThread();
  }
  retVal = sceUsbStart(PSP_USBSTOR_DRIVERNAME, 0, 0);
  if (retVal != 0) {
	  printf("Error starting USB Mass Storage driver (0x%08X)\n", retVal);
	  sceKernelSleepThread();
  }
  retVal = sceUsbstorBootSetCapacity(0x800000);
  if (retVal != 0) {
	  printf("Error setting capacity with USB Mass Storage driver (0x%08X)\n", retVal);
	  sceKernelSleepThread();
  }
  retVal = 0;
}
void usb_deinit(void)
{
  int retVal;
  unsigned int state = sceUsbGetState();
  if (state & PSP_USB_ACTIVATED) {
    retVal = sceUsbDeactivate();
    if (retVal != 0)
      printf("Error calling sceUsbDeactivate (0x%08X)\n",retVal);
  }
  retVal = sceUsbStop(PSP_USBSTOR_DRIVERNAME,0,0);
  if (retVal != 0)
    printf("Error calling sceUsbStop stor (0x%08X)\n",retVal);

  retVal = sceUsbStop(PSP_USBBUS_DRIVERNAME,0,0);
  if (retVal != 0)
    printf("Error calling sceUsbStop bus (0x%08X)\n",retVal);
}

void usb_toggle(void)
{
  unsigned int state = sceUsbGetState();
  if (state & PSP_USB_ACTIVATED) {
    sceUsbDeactivate();
  } else {
    sceUsbActivate(0x1c8);
  }
}
#endif

#if 0 // causes problems, so done in main at the moment
/**
 * Function that is called from _init in kernelmode before the
 * main thread is started in usermode.
 */
__attribute__ ((constructor))
void loaderInit()
{
    pspKernelSetKernelPC();
    pspSdkInstallNoDeviceCheckPatch();
    pspDebugInstallKprintfHandler(NULL);
    pspDebugInstallErrorHandler(MyExceptionHandler);
}
#endif

void cleanupandexit(void)
{
#ifdef USB_ENABLED
  usb_deinit();
#endif
  sceKernelExitGame();
}

/* Exit callback */
void exit_callback(void)
{
  cleanupandexit;
}

/* Callback thread */
void CallbackThread(void *arg)
{
    int cbid;
    cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
    int thid = 0;
    thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
    if (thid >= 0)
	sceKernelStartThread(thid, 0, 0);
    return thid;
}

/* main routine */
int main(int argc, char *argv[])
{
    int stubnum;
#ifdef USB_ENABLED
    pspSdkInstallNoDeviceCheckPatch();
    pspDebugInstallKprintfHandler(NULL);
    pspDebugInstallErrorHandler(MyExceptionHandler);
#endif

    SetupCallbacks();
    // Setup Pad
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(0);

#ifdef USB_ENABLED
    pspDebugScreenInit();
    pspDebugScreenClear();
    usb_init();
#endif

    //get codecStubs
    stubnum = 0;
    CODEC_INITSTUBS 
    codecnum = stubnum;

    pspAudioInit();

    // This is where we call the gui
    gui_main();

    pspAudioEnd();
    cleanupandexit();

    // wait forever
    sceKernelSleepThread();
    return 0;
}
