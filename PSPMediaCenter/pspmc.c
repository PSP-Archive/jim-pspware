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

#include "codec.h"

//  These are the headers for the different codecs
//  auto-generated by the makefile
#include "codecincs.h"

/* Define the module info section */
PSP_MODULE_INFO("PSPMC", 0, 0, 71);

/* Define printf, just to make typing easier */
#define printf  pspDebugScreenPrintf

codecStubs stubs[100];
codecStubs *decoder;
unsigned char banner[] = "PSP Media Center v0.83 by John_K & adresd\0";
int errno, __errno;
int codecnum = 0;

/* Exit callback */
void exit_callback(void)
{
    sceKernelExitGame();
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

    SetupCallbacks();

    // Setup Pad
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(0);

    //get codecStubs
    stubnum = 0;
    CODEC_INITSTUBS codecnum = stubnum;

    pspAudioInit();

    // This is where we call the gui
    gui_main();

    pspAudioEnd();
    sceKernelExitGame();

    // wait forever
    sceKernelSleepThread();
    return 0;
}
