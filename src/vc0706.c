/**
 * \file vc0706.c
 * \brief The core logic for the VC0706 cFS app
 */
#include "vc0706.h"
#include "vc0706_child.h"

vc0706_hk_tlm_t VC0706_HkTelemetryPkt;     /**< The housekeeping telemetry packet for this app */
CFE_SB_PipeId_t VC0706_CommandPipe;        /**< The software bus command pipe for this app */
CFE_SB_MsgPtr_t VC0706MsgPtr;              /**< Used to store a pointer to a message received over the software bus */
uint32 VC0706_ChildTaskID;                 /**< The task ID for VC0706_ChildTask */
VC0706_IMAGE_CMD_PKT_t VC0706_ImageCmdPkt; /**< Struct to store image command for VC0706_SendTimFileName */
led_t led;                                 /**< Represents the LED flash for the camera */

static CFE_EVS_BinFilter_t VC0706_EventFilters[] =
    {
        /* Event ID    mask */
        {VC0706_STARTUP_INF_EID, 0x0000},
        {VC0706_COMMAND_ERR_EID, 0x0000},
        {VC0706_COMMANDNOP_INF_EID, 0x0000},
        {VC0706_COMMANDRST_INF_EID, 0x0000},
};

/**
 * The main entry point for this application
 */
void VC0706_AppMain()
{
    int32 status;
    uint32 RunStatus = CFE_ES_APP_RUN;

    CFE_ES_PerfLogEntry(VC0706_PERF_ID);

    VC0706_AppInit();

    // VC0706 Runloop
    while (CFE_ES_RunLoop(&RunStatus) == TRUE)
    {
        CFE_ES_PerfLogExit(VC0706_PERF_ID);

        /* Pend on receipt of command packet -- timeout set to 500 millisecs */
        status = CFE_SB_RcvMsg(&VC0706MsgPtr, VC0706_CommandPipe, 500); // compilation complains here -- VC0706msgPtr -> VC0706MsgPtr

        CFE_ES_PerfLogEntry(VC0706_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            VC0706_ProcessCommandPacket();
        }
    }

    CFE_ES_ExitApp(RunStatus);
}

/**
 * Application setup. Runs once upon initialization of this application
 */
void VC0706_AppInit()
{
    // Register the app with Executive services
    CFE_ES_RegisterApp();

    // Register the events
    CFE_EVS_Register(VC0706_EventFilters,
                     sizeof(VC0706_EventFilters) / sizeof(CFE_EVS_BinFilter_t),
                     CFE_EVS_BINARY_FILTER);

    // Create the Software Bus command pipe and subscribe to housekeeping messages
    CFE_SB_CreatePipe(&VC0706_CommandPipe, VC0706_PIPE_DEPTH, "VC0706_CMD_PIPE");
    CFE_SB_Subscribe(VC0706_CMD_MID, VC0706_CommandPipe);
    CFE_SB_Subscribe(VC0706_SEND_HK_MID, VC0706_CommandPipe);

    VC0706_ResetCounters();

    VC0706_ChildInit();

    CFE_SB_InitMsg(&VC0706_HkTelemetryPkt,
                   VC0706_HK_TLM_MID,
                   VC0706_HK_TLM_LNGTH, TRUE);

    CFE_EVS_SendEvent(VC0706_STARTUP_INF_EID, CFE_EVS_INFORMATION,
                      "VC0706 App Initialized. Version %d.%d.%d.%d",
                      VC0706_MAJOR_VERSION,
                      VC0706_MINOR_VERSION,
                      VC0706_REVISION,
                      VC0706_MISSION_REV);
}

/**
 * Processes a command received on the VC0706 command pipe
 */
void VC0706_ProcessCommandPacket(void)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_GetMsgId(VC0706MsgPtr);

    switch (MsgId)
    {
    case VC0706_CMD_MID:
        VC0706_ProcessGroundCommand();
        break;

    case VC0706_SEND_HK_MID:
        VC0706_ReportHousekeeping();
        break;

    default:
        VC0706_HkTelemetryPkt.vc0706_command_error_count++;
        CFE_EVS_SendEvent(VC0706_COMMAND_ERR_EID, CFE_EVS_ERROR,
                          "VC0706: invalid command packet,MID = 0x%x", MsgId);
        break;
    }

    return;
}

/** 
 * Processes a ground command received from VC0706_ProcessCommandPacket()
 */
void VC0706_ProcessGroundCommand(void)
{
    uint16 CommandCode;

    CommandCode = CFE_SB_GetCmdCode(VC0706MsgPtr);

    /* Process "known" VC0706 app ground commands */
    switch (CommandCode)
    {
    case VC0706_NOOP_CC:
        VC0706_HkTelemetryPkt.vc0706_command_count++;
        CFE_EVS_SendEvent(VC0706_COMMANDNOP_INF_EID, CFE_EVS_INFORMATION,
                          "VC0706: NOOP command");
        break;

    case VC0706_RESET_COUNTERS_CC:
        VC0706_ResetCounters();
        break;

    /* default case already found during FC vs length test */
    default:
        break;
    }
    return;
}

/** 
 * Gathers the app's telemetry, packetizes it and sends it to the housekeeping task via the software bus.
 * This function is triggered in response to a task telemetry request from the housekeeping task.
 */
void VC0706_ReportHousekeeping(void)
{
    CFE_SB_TimeStampMsg((CFE_SB_Msg_t *)&VC0706_HkTelemetryPkt);
    CFE_SB_SendMsg((CFE_SB_Msg_t *)&VC0706_HkTelemetryPkt);
    return;
}

/**
 * This function resets all the global counter variables that are part of the task telemetry.
 */
void VC0706_ResetCounters(void)
{
    // Status of commands processed by the VC0706 App
    VC0706_HkTelemetryPkt.vc0706_command_count = 0;
    VC0706_HkTelemetryPkt.vc0706_command_error_count = 0;

    CFE_EVS_SendEvent(VC0706_COMMANDRST_INF_EID, CFE_EVS_INFORMATION,
                      "VC0706: RESET command");
}

/**
 * Verifies that a command packet's length is correct
 */
boolean VC0706_VerifyCmdLength(CFE_SB_MsgPtr_t msg, uint16 ExpectedLength)
{
    boolean result = TRUE;

    uint16 ActualLength = CFE_SB_GetTotalMsgLength(msg);

    /*
    ** Verify the command packet length.
    */
    if (ExpectedLength != ActualLength)
    {
        CFE_SB_MsgId_t MessageID = CFE_SB_GetMsgId(msg);
        uint16 CommandCode = CFE_SB_GetCmdCode(msg);

        CFE_EVS_SendEvent(VC0706_LEN_ERR_EID, CFE_EVS_ERROR,
                          "Invalid msg length: ID = 0x%X,  CC = %d, Len = %d, Expected = %d",
                          MessageID, CommandCode, ActualLength, ExpectedLength);
        result = FALSE;
        VC0706_HkTelemetryPkt.vc0706_command_error_count++;
    }

    return (result);
}