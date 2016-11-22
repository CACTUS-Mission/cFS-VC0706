/*******************************************************************************
** File:
**   vc0706_msg.h
**
** Purpose:
**  Define VC0706 App  Messages and info
**
** Notes:
**
**
*******************************************************************************/
#ifndef _vc0706_msg_h_
#define _vc0706_msg_h_

/*
** VC0706 App command codes
*/
#define VC0706_NOOP_CC 0
#define VC0706_RESET_COUNTERS_CC 1

/**
 * A "no arguments" command for this subsystem. Just a header, no command contents.
 */
typedef struct
{
    uint8 CmdHeader[CFE_SB_CMD_HDR_SIZE]; /**< The header of the command packet */

} VC0706_NoArgsCmd_t;

/**
 * A VC0706 housekeeping telemetry packet
 */
typedef struct
{
    uint8 TlmHeader[CFE_SB_TLM_HDR_SIZE];          /**< The header of the packet */
    uint8 vc0706_command_error_count;              /**< The amount of VC0706 command errors to report */
    uint8 vc0706_command_count;                    /**< The amount of VC0706 commands issued */
    char vc0706_filename[VC0706_MAX_FILENAME_LEN]; /**< The filename of the picture taken by the VC0706 application */

} OS_PACK vc0706_hk_tlm_t;

#define VC0706_HK_TLM_LNGTH sizeof(vc0706_hk_tlm_t)

/*************************************************************************/
/*
** Definitions redundantly copied from TIM
** quick and dirty method of sharing this info
*/
#define VC0706_IMAGE_CMD_MID 0x188A  /* This should be == TIM_APP_CMD_MID */
#define VC0706_IMAGE0_CMD_CODE 3     /* This should be == TIM_APP_SEND_IMAGE0_CC */
#define VC0706_IMAGE1_CMD_CODE 5     /* This should be == TIM_APP_SEND_IMAGE1_CC */
#define VC0706_MAX_IMAGE_NAME_LEN 15 /* This should be == TIM_MAX_IMAGE_NAME_LEN */

/**
 * An image command packet. Used to inform the TIM of the filename of a saved image
 */
typedef struct
{
    uint8 CmdHeader[CFE_SB_CMD_HDR_SIZE]; /**< The header of the packet */
    char ImageName[VC0706_MAX_IMAGE_NAME_LEN]; /**< The name of the saved image */
} VC0706_IMAGE_CMD_PKT_t;

#define VC0706_IMAGE_CMD_LNGTH sizeof(VC0706_IMAGE_CMD_PKT_t)

#endif