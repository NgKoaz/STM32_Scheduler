/*
 * scheduler.h
 *
 *  Created on: Oct 29, 2023
 *      Author: HP
 */

#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_

#include <main.h>
#include <scheduler/taskQueue.h>

#define TIMER_CYCLE 	10
#define SCH_MAX_TASKS 	254
#define nullptr 		NULL
#define null			0x00000000

#if defined(SCH_MAX_TASKS) && (SCH_MAX_TASKS > 255)
    #define MY_TYPE uint16_t
#else
    #define MY_TYPE uint8_t
#endif

#define NO_ERROR											0x00
#define ERROR_SCH_TOO_MANY_TASKS 							0x01
#define ERROR_SCH_DELETE_NULL_TASK							0x02
#define ERROR_SCH_WAITING_FOR_SLAVE_TO_ACK					0x04
#define ERROR_SCH_WAITING_FOR_START_COMMAND_FROM_MASTER 	0x08
#define ERROR_SCH_ONE_OR_MORE_SLAVES_DID_NOT_START			0x10
#define ERROR_SCH_LOST_SLAVE								0x20
#define ERROR_SCH_CAN_BUS_ERROR								0x40
#define ERROR_I2C_WRITE_BYTE_AT24C64						0x80

//OPTIONAL DEFINE HERE
#define SCH_REPORT_ERRORS

extern uint8_t TaskIdJustRun;

void 	SCH_Init(void);
void 	SCH_Update(void);
MY_TYPE SCH_Add_Task(void (*pFunc)(void), uint32_t delay, uint32_t period);
uint8_t SCH_Dispatch_Task(void);
uint8_t SCH_Delete_Task(MY_TYPE TaskId);
void 	SCH_Sleep(void);

#endif /* INC_SCHEDULER_H_ */
