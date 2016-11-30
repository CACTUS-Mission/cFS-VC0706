/**
 * \file vc0706_events.h
 * \brief Defines VC0706 app event IDs
 */
#ifndef _vc0706_events_h_
#define _vc0706_events_h_

/** Reserved event ID */
#define VC0706_RESERVED_EID 0
/** Startup information event ID */
#define VC0706_STARTUP_INF_EID 1
/** Command error event ID */
#define VC0706_COMMAND_ERR_EID 2
/** No operation command information event ID */
#define VC0706_COMMANDNOP_INF_EID 3
/** Reset command information event ID */
#define VC0706_COMMANDRST_INF_EID 4
/** Invalid message ID error event ID */
#define VC0706_INVALID_MSGID_ERR_EID 5
/** Message or camera reply length error event ID */
#define VC0706_LEN_ERR_EID 6 // used for checking ingress message length, and reply from camera length
/** Reply error event ID */
#define VC0706_REPLY_ERR_EID 9
/** Child initialization error event ID */
#define VC0706_CHILD_INIT_ERR_EID 7
/** Child initialization event ID */
#define VC0706_CHILD_INIT_EID 8
/** Child initialization information event ID */
#define VC0706_CHILD_INIT_INF_EID 10

#endif