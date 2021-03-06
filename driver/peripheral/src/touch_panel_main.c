/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

/*****************************************************************************
 *
 * Filename:
 * ---------
 *    touch_panel_main.c
 *
 * Project:
 * --------
 *   Maui_Software
 *
 * Description:
 * ------------
 *   This Module defines a task to provide touch panel driver interface
 *
 * Author:
 * -------
 * -------
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * removed!
 * removed!
 * removed!
 * removed!
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#include    "drv_comm.h"
//#include 	"stack_common.h"
#include    "syscomp_config.h"   
#if defined(TOUCH_PANEL_SUPPORT) || defined(HAND_WRITING)
//#include 	"stack_msgs.h"
//#include 	"app_ltlcom.h"       /* Task message communiction */
//#include    "stacklib.h"         /* Basic type for dll, evshed, stacktimer */
//#include    "event_shed.h"       /* Event scheduler */
//#include    "stack_timer.h"     /*Timer*/
//#include    "app_buff_alloc.h"   /*buffer*/
//#include    "task_config.h"      /* Task creation */
//#include    "stack_ltlcom.h"       /*msg_send_ext_queue.....definitions*/
//#include    "uart_sw.h"
//#include    "reg_base.h"
//#include		"eint.h"
#include		"touch_panel.h"
#include 	"touch_panel_buff.h"
//#include    "touch_panel_spi.h"
#include "dcl.h"
//#include "kal_non_specific_general_types.h"
//#include "task_main_func.h"
#include "kal_public_defs.h" //MSBB change #include "stack_config.h"
//#include "kal_release.h"
#include "kal_general_types.h"
#include "kal_public_api.h"
#include "kal_public_defs.h"
#include "drv_features.h"

#if !defined(DRV_TP_NO_EXT_EINT)
#include    "touch_panel_spi.h"
#endif
TouchPanelDataStruct TP;

/*************************************************************************
* Function declaration
 *************************************************************************/
kal_bool    tp_task_create(comptask_handler_struct **handle);

static void tp_task_main(task_entry_struct *task_entry_ptr);
static kal_bool   tp_task_reset(void);

/*************************************************************************
* FUNCTION
*	tp_task_create
*
* DESCRIPTION
*	This function implements xyz entity's create handler.
*
* PARAMETERS
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
kal_bool
tp_task_create(comptask_handler_struct **handle)
{
	static const comptask_handler_struct tp_handler_info =
	{
		tp_task_main,			/* task entry function */
		NULL,			/* task initialization function */
		tp_task_reset		/* task reset handler */
	};

	*handle = (comptask_handler_struct *)&tp_handler_info;
	return KAL_TRUE;
}

/*************************************************************************
* FUNCTION
*	tp_task_main
*
* DESCRIPTION
*	This function implements xyz task's entry function
*
* PARAMETERS
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
void tp_task_main( task_entry_struct * task_entry_ptr )
{	
   kal_uint32  event_group;
   DCL_HANDLE handle;
#if defined(__TOUCH_PANEL_CAPACITY__)
   STS_CTRL_SET_DEVICE_T set_device;
#endif //#if defined(__TOUCH_PANEL_CAPACITY__)

	/*initialize function*/
	if (TP.event == NULL)
      TP.event = kal_create_event_group("TPEVT");
#if !defined(DRV_TP_NO_EXT_EINT) && !defined(__TOUCH_PANEL_CAPACITY__)
	serial_init();
#endif /*!defined(DRV_TP_NO_EXT_EINT)*/
	//touch_panel_init();	
	DclSTS_Initialize();
	#ifdef TOUCH_PANEL_DEBUG
   /*MMI*/   
   touch_panel_test();
   #endif
   //register callback function the follow is RTP.
   handle=DclSTS_Open(DCL_TS,0);

#if defined(__TOUCH_PANEL_CAPACITY__)
   DclSTS_RegisterCallback(handle,DCL_EVNET_STS_DOWN,touch_panel_capacitive_down_hdr);
   DclSTS_RegisterCallback(handle,DCL_EVENT_STS_UP,touch_panel_capacitive_up_hdr);
   kal_set_eg_events(TP.event,2,KAL_OR);
#else
   DclSTS_RegisterCallback(handle,DCL_EVNET_STS_DOWN,touch_panel_down_hdr);
   DclSTS_RegisterCallback(handle,DCL_EVENT_STS_UP,touch_panel_up_hdr);
#endif//#if defined(__TOUCH_PANEL_CAPACITY__)
   DclSTS_Close(handle);	
   //end register.
	while (1) 
	{
       /*lint -e(534)*/kal_retrieve_eg_events(TP.event,3 ,KAL_OR_CONSUME
         ,&event_group,KAL_SUSPEND);	   
      if(event_group & 2) //need to execute after file system task initial
      {
#if defined(__TOUCH_PANEL_CAPACITY__)
         handle=DclSTS_Open(DCL_TS,0);
         set_device.mode = DCL_CTP_FIRMWARE_UPDATE;
         DclSTS_Control(handle,STS_CMD_SET_DEVICE,(DCL_CTRL_DATA_T *)&set_device);
         DclSTS_Close(handle);
#endif /*#if defined(__TOUCH_PANEL_CAPACITY__)*/      	
      }
      if( (event_group & 1) && (TP.state==DOWN) )
      {
         //touch_panel_down_hdr();
         TP.tp_down_cb(NULL);
      }
      else if(event_group & 1)/*TP.state==UP*/
      {
         //touch_panel_up_hdr();
         TP.tp_up_cb(NULL);
      }      

	}

	/* Do component task's processing here */
}

/*************************************************************************
* FUNCTION
*	tp_task_reset
*
* DESCRIPTION
*	This function implements xyz's reset handler
*
* PARAMETERS
*	task_index	-	task's index
*
* RETURNS
*
* GLOBALS AFFECTED
*
*************************************************************************/
kal_bool
tp_task_reset(void)
{
	/* Do task's reset here.
	 * Notice that: shouldn't execute modules reset handler since
	 * stack_task_reset() will do. */
	return KAL_TRUE;
}

#endif
