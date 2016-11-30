/**
 * \file vc0706_child.c
 * \brief The child task spawned by the VC0706 app to take pictures
 */
#include "vc0706_child.h"
#include "vc0706_device.h"

// Command packet from vc0706.c
extern VC0706_IMAGE_CMD_PKT_t VC0706_ImageCmdPkt;

char *taskName = "VC0706 Child Task"; /**< Name under which to register this task */

/**
 * Child Task Initialization function
 *
 * This function is invoked during VC0706 application startup initialization to create and initialize the Camera Child Task.
 * The purpose for the child task is to send commands to the camera, and to recieve then store images taken.
 * \returns The success value of the child task creation function call
 * \sa #VC0706_AppInit
 */
int VC0706_ChildInit(void)
{
    // Read number of reboots
    VC0706_setNumReboots();

    // Create child task - VC0706 monitor task
    int32 result = CFE_ES_CreateChildTask(&VC0706_ChildTaskID,
                                          VC0706_CHILD_TASK_NAME,
                                          (void *)VC0706_ChildTask, 0,
                                          VC0706_CHILD_TASK_STACK_SIZE,
                                          VC0706_CHILD_TASK_PRIORITY, 0);
    if (result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(VC0706_CHILD_INIT_ERR_EID, CFE_EVS_ERROR,
                          "%s initialization error: create task failed: result = %d",
                          taskName, (int)result);
    }
    else
    {
        CFE_EVS_SendEvent(VC0706_CHILD_INIT_EID, CFE_EVS_INFORMATION,
                          "%s initialization info: create task complete: result = %d",
                          taskName, (int)result);
    }
    return result;
}

/**
 * The entry point for the VC0706 application child task.
 * Registers with CFE as a child task, interfaces with the parent task and calls the child task main loop function.
 * Should the main loop function return due to a breakdown in the interface handshake with the parent task,
 * this function will self delete as a child task with CFE.
 */
void VC0706_ChildTask(void)
{
    // The child task runs until the parent dies (normal end) or until it encounters a fatal error (semaphore error, etc.)...
    int32 result = CFE_ES_RegisterChildTask();

    if (result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(VC0706_CHILD_INIT_ERR_EID, CFE_EVS_ERROR,
                          "%s initialization error: register child failed: result = %d",
                          taskName, (int)result);
    }
    else
    {
        CFE_EVS_SendEvent(VC0706_CHILD_INIT_EID, CFE_EVS_INFORMATION,
                          "%s initialization complete", taskName);

        // Child task process loop
        Camera_t cam_one;
        Camera_t cam_two;
        // Attempt to initialize the cameras on /dev/ttyama0 and /dev/ttyama1
        if (init(&cam_one, 0) == -1 || init(&cam_two, 1) == -1) // Error
        {
            OS_printf("Camera initialization error.\n");
        } else
        {
            // Pass the cameras in an array to VC0706_takePics()
            Camera_t cameras[] = {cam_one, cam_two};
            VC0706_takePics(cameras, 2);
        }
    }

    // This call allows cFE to clean-up system resources
    CFE_ES_ExitChildTask();
}

/** 
 * Sends the filename of a saved picture to TIM.
 * \returns At the moment, 0.
 */
int VC0706_SendTimFileName(char *file_name)
{
    // Initialize a Software Bus message using our packet templates for this app
    CFE_SB_InitMsg((void *)&VC0706_ImageCmdPkt, (CFE_SB_MsgId_t)VC0706_IMAGE_CMD_MID, (uint16)VC0706_IMAGE_CMD_LNGTH, (boolean)1);

    int32 ret = 0;

    // Determine which camera is sending data, for Command Code determintation
    if (file_name[4] == '0')
    {
        ret = CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&VC0706_ImageCmdPkt, (uint16)VC0706_IMAGE0_CMD_CODE);
    }
    else if (file_name[4] == '1')
    {
        ret = CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&VC0706_ImageCmdPkt, (uint16)VC0706_IMAGE1_CMD_CODE);
    }
    else
    {
        OS_printf("\tDid not recognize camera identifier in filename. Defaulting to Cam 0\n");
        ret = CFE_SB_SetCmdCode((CFE_SB_MsgPtr_t)&VC0706_ImageCmdPkt, (uint16)VC0706_IMAGE0_CMD_CODE);
    }

    if (ret < 0)
    {
        OS_printf("VC0706: SendTimFileName() Set Cmd Code Ret [%d].\n", (int)ret);
    }

    snprintf(VC0706_ImageCmdPkt.ImageName, sizeof(VC0706_ImageCmdPkt.ImageName), "%s", file_name);

    CFE_SB_GenerateChecksum((CFE_SB_MsgPtr_t)&VC0706_ImageCmdPkt);

    CFE_SB_SendMsg((CFE_SB_Msg_t *)&VC0706_ImageCmdPkt);

    CFE_EVS_SendEvent(VC0706_CHILD_INIT_INF_EID, CFE_EVS_INFORMATION, "Message sent to TIM from VC0706.");

    return 0;
}