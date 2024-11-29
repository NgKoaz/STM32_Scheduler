/*
 * scheduler.c
 *
 *  Created on: Oct 29, 2023
 *      Author: HP
 */

#include <scheduler/scheduler.h>


void SCH_Init(void){
	initQueue();
}

void SCH_Update(void){
	update();
}

MY_TYPE SCH_Add_Task(void (*pFunc)(void), uint32_t delay, uint32_t period) {
	return createTask(pFunc, delay, period);
}

uint8_t SCH_Dispatch_Task(void) {
	return dispatchTask();
}

uint8_t SCH_Delete_Task(MY_TYPE TaskId) {
	return deleteTask(TaskId);
}

void SCH_Sleep(void){
	sleep();
}

