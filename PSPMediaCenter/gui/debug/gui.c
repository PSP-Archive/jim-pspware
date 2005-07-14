/*********************************************************************
* 
*  Debug based gui for PSP Media Center
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
#include <psphprm.h>

#include "../../codec.h"

//  These are the headers for the different codecs
//  auto-generated by the makefile
#include "../../codecincs.h"

/* Define printf, just to make typing easier */
#define printf  pspDebugScreenPrintf

// Common externs
extern unsigned char banner[];
extern codecStubs stubs[100];
extern int errno, __errno;
extern int codecnum;

static codecStubs *decoder;
static int loopmode;

static void strcat2(char *dest, char *src)
{
  int pos = 0;
  int pos2 = 0;
  while (*(dest + pos2) != 0)
    pos2++;
  while (*(src + pos) != 0) {
    *(dest + pos2) = *(src + pos);
    pos++;
    pos2++;
  }
  *(dest + pos2) = 0;
}

static int forceskip;
static void playmedia(char *rootpath, char *modname)
{
  char filename[200];
  u32 buttonsold;
  SceCtrlData pad;
  u32 remoteButtons, remoteButtonsOld = 0;
  int codec;
  int finished = 0;

  filename[0] = 0;
  strcat2(filename, rootpath);
  strcat2(filename, modname);

  pspDebugScreenClear();
  printf("%s\n\n", banner);
  printf("loading media File : %s\n", modname);

  //determine codec of the file
  for (codec = 0; codec <= codecnum; codec++) {
    char *ptr = &(stubs[codec].extension[0]);
    while (*ptr != 0) {
      if (strncasecmp(&modname[strlen(modname) - 3], ptr, 3) == 0) {
        decoder = &stubs[codec];
        break;
      }
      ptr += 4;
    }
  }

  decoder->init(0);
  if (decoder->load(filename)) {
    decoder->play();

    pspDebugScreenSetXY(0, 32);
    printf("X = Pause/Resume.  START = Tune Select.  SELECT = Exit.\n");
    pspDebugScreenSetXY(0, 26);
    printf("Playing\n\n");

    forceskip = 0;

    sceCtrlReadBufferPositive(&pad, 1);
    sceHprmPeekCurrentKey(&remoteButtons);

    buttonsold = pad.Buttons;
    remoteButtonsOld = remoteButtons;
    while (finished == 0) {
      sceDisplayWaitVblankStart();
      sceCtrlReadBufferPositive(&pad, 1);
      sceHprmPeekCurrentKey(&remoteButtons);

      // Check if we have hit the end of tune
      if (loopmode != 0) {
        if (decoder->eos != 0) {
          if (decoder->eos() == 1) { // End of file, advance in playlist
            pad.Buttons = PSP_CTRL_RTRIGGER;
          }
        }
      }

      if (pad.Buttons != buttonsold) {
        if (pad.Buttons & PSP_CTRL_LTRIGGER) {	// Previous tune
          forceskip = 1;
          finished = 1;
        }
        if (pad.Buttons & PSP_CTRL_RTRIGGER) {	// Next tune
          forceskip = 2;
          finished = 1;
        }
        if (pad.Buttons & PSP_CTRL_TRIANGLE)
          loopmode = !loopmode;
        if (pad.Buttons & PSP_CTRL_CIRCLE)
          decoder->stop();
        if (pad.Buttons & PSP_CTRL_CROSS)
          decoder->pause();
        if (pad.Buttons & PSP_CTRL_START)
          finished = 1;
        if (pad.Buttons & PSP_CTRL_SELECT)
          finished = 2;
        buttonsold = pad.Buttons;
      }
      if (remoteButtons != remoteButtonsOld && !(remoteButtons & PSP_HPRM_HOLD)) {
        if (remoteButtons & PSP_HPRM_PLAYPAUSE)
          decoder->pause();
        if (remoteButtons & PSP_HPRM_BACK) {
          forceskip = 1;
          finished = 1;
        }
        if (remoteButtons & PSP_HPRM_FORWARD) {
          forceskip = 2;
          finished = 1;
        }
        remoteButtonsOld = remoteButtons;
      }
      //  Show loop status
      pspDebugScreenSetXY(58, 1);
      if (loopmode == 0) {
        printf(" LOOP  ");
      } else {
        printf("ADVANCE");
      }
      //  Show the time
      if (decoder->time != NULL) {
        char time[200];
        decoder->time(time);
        pspDebugScreenSetXY(58, 0);
        printf("%s", time);
        pspDebugScreenSetXY(0, 32);
      }
    }
    decoder->stop();
    decoder->end();
    if (finished == 2) {
      pspAudioEnd();
      sceKernelExitGame();
    }
  }
}


typedef struct {
  char *filename;
  char *pathname;
} files_infot_t;

#define MAX_FILE_NUM  10000
static files_infot_t files_info[MAX_FILE_NUM];
static int files_infonum;

static void sortmedialist()
{
  int found = 1;
  int swap;
  int count;
  char *temp;
  while (found == 1) {
    found = 0;
    for (count = 0; count < (files_infonum - 1); count++) {
      swap = 0;
      if (strcmp(files_info[count].filename,files_info[count+1].filename) > 0) {
        temp = files_info[count].filename;
        files_info[count].filename = files_info[count + 1].filename;
        files_info[count + 1].filename = temp;
        temp = files_info[count].pathname;
        files_info[count].pathname = files_info[count + 1].pathname;
        files_info[count + 1].pathname = temp;
        found = 1;
      }
    }
  }
}

static SceIoDirent direnta;
static void addmedialistpath(char *path)
{
  int dirid;
  int retval;
  int x;
  int found;
  SceIoDirent *direntt;
  if ((dirid = sceIoDopen(path)) > 0) {	//  Opened ok
    retval = 1;
    //direntt = (SceIoDirent *)malloc(sizeof(SceIoDirent));
    direntt = (SceIoDirent *)&direnta;
    while ((retval > 0) && (files_infonum < MAX_FILE_NUM)) {
      retval = sceIoDread(dirid, direntt);
      if (retval > 0) {
        sceKernelDcacheWritebackAll();
        if ((direntt->d_stat.st_attr & 0x10) == 0x10) { // directory
          if (direntt->d_name[0] != '.') {
            char *pathname = (char *)malloc(200);
            sprintf(pathname,"%s%s/",path,direntt->d_name);
            addmedialistpath(pathname);
            //free(pathname); // dont free, as used in files
          }
        }
//        else if (((direntt->d_stat.st_attr & 0x20) == 0x20) && (direntt->d_name[0] != 0)) { // File
        else if  (direntt->d_name[0] != 0) { // File
          // Only add files of types known to codecs loaded
          // Now check against the codecs known to us
          found = 0;
          for (x = 0; x < codecnum; x++) {
            char *ptr = stubs[x].extension;
            while (*ptr != 0) {
              if (strncasecmp(&direntt->d_name[strlen(direntt->d_name) - 3], ptr, 3) == 0)
                found = 1;
              ptr+=4;
            }
          }
          if (found == 1) {
            files_info[files_infonum].filename = (char *) malloc(200);
            memcpy(files_info[files_infonum].filename, direntt->d_name, 200);
            files_info[files_infonum].pathname = path;
            files_infonum++;
          } // if found
        } // if d_name
      } // if retval
      //free(direntt);
    } // while
    sceIoDclose(dirid);
  } // if
}
static void fillmedialist(char *path)
{
  files_infonum = 0;
  addmedialistpath(path);
  sortmedialist();
}

static int selectmedia()
{
  SceCtrlData pad;
  static int highlight = 0;
  int highlightold;
  int finished = 0;
  int count;
  int x, y;
  u32 buttonsold = 0;
  int basepos;

  if (forceskip != 0) {	// we are forcing a skip
    if (forceskip == 1) {	// previous tune
      if (highlight != 0)
        highlight--;
    } else if (forceskip == 2) {	// next tune
      if (highlight < (files_infonum-1))
        highlight++;
      else 
        highlight = 0; // move back to top of list
    }
    forceskip = 0;
    return highlight;
  }

  printf("Select media to play:\n\n");
  // Save screen position
  x = pspDebugScreenGetX();
  y = pspDebugScreenGetY();

  sceCtrlReadBufferPositive(&pad, 1);
  buttonsold = pad.Buttons;

  pspDebugScreenSetXY(0, 32);
  printf("Up/Down = Move cursor.  X = Select.  SELECT = Exit.\n");

  highlightold = -1;
  while (finished == 0) {	// Draw the menu firstly
    sceDisplayWaitVblankStart();
    //  Show loop status
    pspDebugScreenSetXY(58, 1);
    if (loopmode == 0) {
      printf(" LOOP  ");
    } else {
      printf("ADVANCE");
    }
    if (highlightold != highlight) {
      // Calc position in the list, given number of files and highlight position
      if (highlight < 11)
        basepos = 0;
      else		//  we must scroll
        basepos = highlight - 11;

      pspDebugScreenSetXY(x, y);
      for (count = basepos; count < basepos + 22; count++) {
        if (count >= files_infonum)
          printf("\n");
        else {
          if (highlight == count)
            printf("-> %02d - %-50s \n", count, files_info[count].filename);
          else
            printf("   %02d - %-50s \n", count, files_info[count].filename);
        }
      }
    }
    highlightold = highlight;
    // Now read the keys and act appropriately
    sceCtrlReadBufferPositive(&pad, 1);

    if (buttonsold != pad.Buttons) {
      if (pad.Buttons & PSP_CTRL_RIGHT)
        if (highlight < (files_infonum - 11))
          highlight += 10;
        else
          highlight = files_infonum - 1;
      if (pad.Buttons & PSP_CTRL_LEFT)
        if (highlight >= 10)
          highlight -= 10;
        else
          highlight = 0;
      if (pad.Buttons & PSP_CTRL_UP)
        if (highlight >= 1)
          highlight--;
      if (pad.Buttons & PSP_CTRL_DOWN)
        if (highlight < (files_infonum - 1))
          highlight++;
      if (pad.Buttons & PSP_CTRL_TRIANGLE)
        loopmode = !loopmode;
      if (pad.Buttons & PSP_CTRL_CROSS)
        return highlight;
      if (pad.Buttons & PSP_CTRL_SELECT)
        return -1;
    }
    buttonsold = pad.Buttons;
  }
}

static void getproperpath(char *dest, char *src)
{
  int pos;
  int found = -1;
  pos = 0;
  while (*(src + pos) != 0) {
    if (*(src + pos) == '/')
      found = pos;
    *(dest + pos) = *(src + pos);
    pos++;
  }
  if (found != -1)
    *(dest + found + 1) = 0;
}

/* main routine */
int gui_main(void)
{
  char rootpath[200];
  int filenum;
  int stubnum;

  pspDebugScreenInit();
  pspDebugScreenClear();
  sprintf(rootpath, "ms0:/PSP/MUSIC/");

  fillmedialist(rootpath);

  loopmode = !0;
  //  Loop around, offering a mod, till they cancel
  filenum = 1;
  forceskip = 0;
  while (filenum >= 0) {
    // Setup screen  so it doesnt get messy
    pspDebugScreenClear();
    printf("%s\n\n", banner);

    { //  Print out a list of the file extensions supported
      int c;
      printf("filetypes : ");
      for (c = 0; c < codecnum; c++) {
        unsigned char *ptr = stubs[c].extension;
        while (*ptr != 0) {
          printf("%s ", ptr);
          ptr += 4;
        }
      }
      printf("\n\n");
    }

    // Filetype list
    printf("Media Path: %s\n\n", rootpath);

    filenum = selectmedia();
    if (filenum >= 0) {
      playmedia(files_info[filenum].pathname,files_info[filenum].filename);
    }
  }
  return 0;
}
