/******************************************************************************
  Copyright (C), 2001-2011, ECI Tech. Co., Ltd.
 ******************************************************************************
  File Name     : kgen_platform.h
  Version       : Initial Draft
  Author        : huhuifeng
  Created       : 2011/5/30
  Last Modified :
  Description   : kernel platoform common header file
  Function List :
  History       :
  1.Date        : 2011/5/30
    Author      : huhuifeng
    Modification: Created file

******************************************************************************/


#ifndef __KGEN_PLATFORM_H__
#define __KGEN_PLATFORM_H__

#define printf printk

#define OK 0
#define ERROR (-1)

#include "kgen_malloc.h"
#include "kgen_thrd.h"
#include "kgen_mute.h"
#include "kgen_msgq.h"

/* 
delay for ms.  the task will sleep
if a signal occurs 
*/
 int kgen_thread_sleep_ms(unsigned int uiMsTime);
 

/* 
delay for ms.  the task will not sleep and is in running when delay, other tasks will 
not be excuted
*/
 void kgen_thread_delay_ms(unsigned int uiMsTime);
 

/* 
delay for us.  the task will not sleep and is in running when delay, other tasks will 
not be excuted
*/
 void kgen_thread_delay_us(unsigned int uiUsTime);
 


#endif




