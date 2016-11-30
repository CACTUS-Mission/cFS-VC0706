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

/** Baud rate to use on the serial interface for the camera */
#define BAUD 38400
/** The code that signals the beginning of a command */
#define COMMAND_BEGIN 0x56
/** The code returned after a successful operation */
#define COMMAND_SUCCESS 0x76
/** The reset command code */
#define RESET 0x26
/** The version of camera we're using */
#define GEN_VERSION 0x11
/** The read frame buffer command code */
#define READ_FBUF 0x32
/** The get frame buffer length command code */
#define GET_FBUF_LEN 0x34
/** The frame buffer control command code */
#define FBUF_CTRL 0x36
/** The downsize control command code */
#define DOWNSIZE_CTRL 0x54
/** The downsize status command code */
#define DOWNSIZE_STATUS 0x55
/** The read data command code */
#define READ_DATA 0x30
/** The write data command code */
#define WRITE_DATA 0x31
/** The motion detection communication control command code */
#define COMM_MOTION_CTRL 0x37
/** The motion detection communication status command code */
#define COMM_MOTION_STATUS 0x38
/** The motion detected code */
#define COMM_MOTION_DETECTED 0x39
/** The motion detection control command code */
#define MOTION_CTRL 0x42
/** The motion detection status command code */
#define MOTION_STATUS 0x43
/** The TV out control command code */
#define TVOUT_CTRL 0x44
/** The on-screen-display add character command code */
#define OSD_ADD_CHAR 0x45
/** The stop current frame command code */
#define STOPCURRENTFRAME 0x0
/** The stop next frame command code */
#define STOPNEXTFRAME 0x1
/** The resume frame command code */
#define RESUMEFRAME 0x3
/** The step frame command code */
#define STEPFRAME 0x2
/** The code for setting the image size to 640p */
#define SIZE640 0x00
/** The code for setting the image size to 320p */
#define SIZE320 0x11
/** The code for setting the image size to 160p */
#define SIZE160 0x22
/** The command code for setting the zoom */
#define SET_ZOOM 0x52
/** The command code for getting the zoom */
#define GET_ZOOM 0x53
/** The size of the camera buffer */
#define CAMERABUFFSIZ 100
/** The delay on the camera */
#define CAMERADELAY 10
/** The scale of all the timeouts for this app (add to this to slow down sample rates, subtract to speed up) */
#define TO_SCALE 1
/** The default timeout in microseconds */
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