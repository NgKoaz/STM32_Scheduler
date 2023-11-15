/*
 * scheduler.h
 *
 *  Created on: Oct 29, 2023
 *      Author: HP
 */

#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_

#include "main.h"

#define MAX_TASK 	10
#define nullptr 	NULL
#define null		0x00000000

//OPTIONAL DEFINE HERE
//#define SCH_REPORT_ERRORS


typedef struct {
	void (*pFunc)(void);
	uint32_t	Delay;
	uint32_t 	Period;
	uint8_t 	PreTaskID;
	uint8_t 	NextTaskID;
} sTask;

void 	SCH_Init(void);
void 	SCH_Update(void);
uint8_t SCH_Add_Task(void (*pFunc)(void), uint32_t Delay, uint32_t Period);
void 	SCH_Dispatch_Tasks(void);
uint8_t SCH_Delete_Task(uint8_t TaskID);
void 	SCH_Report_Status(void);

#endif /* INC_SCHEDULER_H_ */
