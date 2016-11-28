/**
 * \file vc0706_child.c
 * \brief The child task spawned by the VC0706 app to take pictures
 */
#include "vc0706_child.h"

char num_reboots[3];

// Command packet from vc0706.c
extern VC0706_IMAGE_CMD_PKT_t VC0706_ImageCmdPkt;

// From vc0706_device.c
extern int VC0706_takePics(void);

char *taskName = "VC0706 Child Task"; /**< Name under which to register this task */

/**
 * Initialization function for the VC0706 child task
 * \returns The completion status returned by the CFE child task creation call
 */
int VC0706_ChildInit()
{
    // Read number of reboots
    setNumReboots();

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
                          taskName, result);
    }
    else
    {
        CFE_EVS_SendEvent(VC0706_CHILD_INIT_EID, CFE_EVS_INFORMATION,
                          "%s initialization info: create task complete: result = %d",
                          taskName, result);
    }
    return result;
}

/**
 * The main function called by CFE after its registry as an Executive Services child task.
 */
void VC0706_ChildTask()
{
    // The child task runs until the parent dies (normal end) or until it encounters a fatal error (semaphore error, etc.)...
    int32 result = CFE_ES_RegisterChildTask();

    if (result != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(VC0706_CHILD_INIT_ERR_EID, CFE_EVS_ERROR,
                          "%s initialization error: register child failed: result = %d",
                          taskName, result);
    }
    else
    {
        CFE_EVS_SendEvent(VC0706_CHILD_INIT_EID, CFE_EVS_INFORMATION,
                          "%s initialization complete", taskName);

        // Child task process loop
        VC0706_takePics();
    }

    /* This call allows cFE to clean-up system resources */
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
        OS_printf("VC0706: SendTimFileName() Set Cmd Code Ret [%d].\n", ret);
    }

    snprintf(VC0706_ImageCmdPkt.ImageName, sizeof(VC0706_ImageCmdPkt.ImageName), "%s", file_name);

    CFE_SB_GenerateChecksum((CFE_SB_MsgPtr_t)&VC0706_ImageCmdPkt);

    CFE_SB_SendMsg((CFE_SB_Msg_t *)&VC0706_ImageCmdPkt);

    CFE_EVS_SendEvent(VC0706_CHILD_INIT_INF_EID, CFE_EVS_INFORMATION, "Message sent to TIM from VC0706.");

    return 0;
}

/**
 * Reads the number of reboots since mission start (from /ram/logs/reboot.txt file)
 */
void setNumReboots()
{
    // Reset all of the characters in num_reboots to NULL
    memset(num_reboots, '0', sizeof(num_reboots));

    // Define file permissions for OS_open - this is required in case the file doesn't exist.
    //       Owner write | Owner read | Group read | Group write | Others read
    mode_t mode = S_IWRITE | S_IREAD | S_IRGRP | S_IWGRP | S_IROTH;

    // Open the system's reboot log
    int32 fd = OS_open((const char *)"/ram/logs/reboot.txt", (int32)OS_READ_ONLY, (uint32)mode);

    // Check for file open success
    if (fd != OS_FS_SUCCESS)
        OS_printf("\tCould not open reboot file in VC, ret = %d!\n", fd);

    // Read three characters from file into num_reboots
    int os_ret = OS_read(fd, (void *)num_reboots, 3);
    // Make sure the above read completed successfully
    if (os_ret != OS_FS_SUCCESS)
    {
        // If not, set all num_reboots values to 9
        memset(num_reboots, '9', sizeof(num_reboots));
        OS_printf("\tCould not read from reboot file in VC, ret = %d!\n", os_ret);
    }

    // Close the file handle (for /ram/logs/reboot.txt)
    OS_close(fd);
}
