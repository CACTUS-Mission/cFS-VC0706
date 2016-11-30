/**
 * \file vc0706_core.h
 * \brief Header for serial communication with the VC0706 camera
 * Retrieved from https://github.com/vyykn/VC0706
 *
 * Edited By Zach Richard for use on TRAPSat aboard the RockSat-X 2016 Mission
 * Edited by Ezra Brooks for use on TRAPSat aboard the CACTUS-1 Mission
 */
#ifndef _vc0706_core_h_
#define _vc0706_core_h_

#include "vc0706.h"

// Brian's Defines
#define BAUD 38400

#define SERIAL_NUMBER 0x00
#define COMMAND_BEGIN 0x56
#define COMMAND_SUCCESS 0x76
#define RESET 0x26
#define GEN_VERSION 0x11
#define READ_FBUF 0x32
#define GET_FBUF_LEN 0x34
#define FBUF_CTRL 0x36
#define DOWNSIZE_CTRL 0x54
#define DOWNSIZE_STATUS 0x55
#define READ_DATA 0x30
#define WRITE_DATA 0x31
#define COMM_MOTION_CTRL 0x37
#define COMM_MOTION_STATUS 0x38
#define COMM_MOTION_DETECTED 0x39
#define MOTION_CTRL 0x42
#define MOTION_STATUS 0x43
#define TVOUT_CTRL 0x44
#define OSD_ADD_CHAR 0x45

#define STOPCURRENTFRAME 0x0
#define STOPNEXTFRAME 0x1
#define RESUMEFRAME 0x3
#define STEPFRAME 0x2

#define SIZE640 0x00
#define SIZE320 0x11
#define SIZE160 0x22

#define MOTIONCONTROL 0x0
#define UARTMOTION 0x01
#define ACTIVATEMOTION 0x01

#define SET_ZOOM 0x52
#define GET_ZOOM 0x53

#define CAMERABUFFSIZ 100
#define CAMERADELAY 10
#define TO_SCALE 1
#define TO_U 200000

/**
 * Represents a VC0706 camera attached via serial
 */
typedef struct Camera_t {
    bool motion; /**< Whether or not motion detection is enabled */
    bool ready; /**< Whether or not the serial interface has been initialized */
    int ttyInterface; /**< The ID of the tty interface. i.e. 0 for /dev/ttyAMA0 */
    int fd; /**< The handle for the serial connection */

    int frameptr; /**< Points to the next frame in the buffer during large read operations */
    int bufferLen; /**< Length of the camera's buffer */
    int serialNum; /**< Serial number of the camera. Used for sending commands */
    char imageName[OS_MAX_PATH_LEN]; /**< Name of the saved image. Uses OSAL's max path length macro to define its length */
} Camera_t;


int init(Camera_t *cam, uint8 ttyInterface);
bool checkReply(Camera_t *cam, int cmd, int size);
void clearBuffer(Camera_t *cam);
void reset(Camera_t *cam);
void resumeVideo(Camera_t *cam);
int  getVersion(Camera_t *cam);
void setMotionDetect(Camera_t *cam, bool flag);
char * takePicture(Camera_t *cam, char * file_path);
void sendCommand(Camera_t *cam, uint8_t cmd, uint8_t args[], uint8_t argLen);

#endif