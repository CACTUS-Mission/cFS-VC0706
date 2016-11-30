/**
 * \file vc0706.h
 * \brief Main header for the VC0706 app
 */
#ifndef _vc0706_h_
#define _vc0706_h_


// Required header files
#include "cfe.h"
#include "cfe_error.h"
#include "cfe_evs.h"
#include "cfe_sb.h"
#include "cfe_es.h"

// App specific files
#include <time.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wiringSerial.h>
#include <wiringPi.h>

/** The depth for the command pipe (maximum number of commands stored in the buffer) */
#define VC0706_PIPE_DEPTH 32
/** Name for the VC0706 CFE child task */
#define VC0706_CHILD_TASK_NAME "CAMERA_CONTROL"
/** Number of bytes to allocate for the child task's stack (8KB) */
#define VC0706_CHILD_TASK_STACK_SIZE 8192
/** The CFE priority for the child task */
#define VC0706_CHILD_TASK_PRIORITY 200
/** GPIO pin for LEDs */
#define LED_PIN 16
/** Maximum expected filename length /ram/images/<reboots [3 char]>_<cam 0 or 1 [1 char]>_<filenum [3 char]>.jpg */
#define VC0706_MAX_FILENAME_LEN 24

// This application's component headers
#include "vc0706_perfids.h"
#include "vc0706_msgids.h"
#include "vc0706_msg.h"
#include "vc0706_events.h"
#include "vc0706_version.h"

#include "vc0706_led.h"
#include "vc0706_core.h"

void VC0706_AppMain(void);
void VC0706_AppInit(void);
void VC0706_ProcessCommandPacket(void);
void VC0706_ProcessGroundCommand(void);
void VC0706_ReportHousekeeping(void);
void VC0706_ResetCounters(void);

boolean VC0706_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength);

#endif