/*
 * taskQueue.h
 *
 *  Created on: Nov 29, 2024
 *      Author: HP
 */

#ifndef INC_SCHEDULER_TASKQUEUE_H_
#define INC_SCHEDULER_TASKQUEUE_H_

#include "scheduler/scheduler.h"

#if defined(SCH_MAX_TASKS) && (SCH_MAX_TASKS > 255)
    #define MY_TYPE uint16_t
#else
    #define MY_TYPE uint8_t
#endif

typedef struct {
	void (*pFunc)(void);
	uint32_t	delay;
	uint32_t 	period;
	MY_TYPE 	previousId;
	MY_TYPE 	nextId;
} sTask;

typedef struct {
	MY_TYPE nextId;
} IdleID;


void initQueue(void);
MY_TYPE createTask(void (*pFunc)(void), uint32_t delay, uint32_t period);
uint8_t deleteTask(MY_TYPE deletedTaskId);
uint8_t dispatchTask(void);
void update(void);
void sleep(void);


#endif /* INC_SCHEDULER_TASKQUEUE_H_ */
