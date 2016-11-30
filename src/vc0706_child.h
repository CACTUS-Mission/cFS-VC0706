/**
 * \file vc0706_child.h
 * \brief The header for the VC0706 child task
 */
#ifndef _vc0706_child_h_
#define _vc0706_child_h_

/*
** VC0706 Master Header
*/
#include "vc0706.h"

//TODO: Check necessity of each of these to trim fat

extern vc0706_hk_tlm_t VC0706_HkTelemetryPkt;
extern uint32 VC0706_ChildTaskID;

int VC0706_ChildInit();
void VC0706_ChildTask();
int VC0706_SendTimFileName(char *file_name);

#endif