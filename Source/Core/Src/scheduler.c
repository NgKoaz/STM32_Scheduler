/*
 * scheduler.c
 *
 *  Created on: Oct 29, 2023
 *      Author: HP
 */

#include "scheduler.h"

#define TIMER_CYCLE 10

short head_taskID;
sTask list_task[SCH_MAX_TASKS + 1]; //Zero index is not used.
uint16_t time_skip;
uint16_t count_task;

uint8_t TaskIdJustRun = 0;

uint8_t errorPort;
uint8_t errorCode;
uint8_t lastErrorCode;
uint32_t errorCount;

uint8_t getTaskId(){
	for (uint16_t i = 1; i < SCH_MAX_TASKS + 1; i++)
		if (list_task[i].pFunc == nullptr)
			return i;
	//Throw Error
	return 0;
}

void SCH_Init(void){
	head_taskID = null;
	time_skip = 0;
	count_task = 0;
	errorCode = 0;
	lastErrorCode = 0;
	errorCount = 30000 / TIMER_CYCLE;

	for (short i = 0; i < SCH_MAX_TASKS + 1; i++){
		list_task[i].pFunc      = nullptr;
		list_task[i].NextTaskID = null;
	}
}

void SCH_Update(void){
	if (head_taskID == null) {
		time_skip = (count_task > 0) ? time_skip + 1 : 0;
		return;
	}
	time_skip++;
	if (list_task[head_taskID].Delay > 0){
		int temp = list_task[head_taskID].Delay - time_skip;
		if (temp >= 0) {
			list_task[head_taskID].Delay = temp;
			time_skip = 0;
		} else {
			list_task[head_taskID].Delay = 0;
			time_skip = 0 - temp;
		}
	}
}

uint8_t Create_Task(void (*pFunc)(void), uint32_t Delay, uint32_t Period){
	if (count_task > SCH_MAX_TASKS){
		errorCode = ERROR_SCH_TOO_MANY_TASKS;
		return SCH_MAX_TASKS;
	}
	uint8_t newId = getTaskId();
	//Throw Error
	if (newId == 0) return -1;

	Delay  /= TIMER_CYCLE;
	Period /= TIMER_CYCLE;

	list_task[newId].pFunc			= pFunc;
	list_task[newId].Delay 		= Delay + time_skip;
	list_task[newId].Period	    = Period;
	list_task[newId].NextTaskID = null;
	count_task++;
	return newId;
}

uint8_t Enqueue_Task(uint8_t newTaskID){
	//First task in queue.
	if (head_taskID == null){
		head_taskID = newTaskID;
		return newTaskID;
	}

	//Task will add at middle of queue.
	uint8_t ini_task = head_taskID;
	uint8_t pre_task = null;
	uint32_t sum = 0;
	while (ini_task != null){
		if (sum + list_task[ini_task].Delay > list_task[newTaskID].Delay){
			if (ini_task == head_taskID){
				list_task[newTaskID].NextTaskID = head_taskID;
				head_taskID = newTaskID;
				list_task[ini_task].Delay -= list_task[newTaskID].Delay;
			} else {
				list_task[newTaskID].NextTaskID = ini_task;
				list_task[pre_task].NextTaskID = newTaskID;
				list_task[newTaskID].Delay -= sum;
				list_task[ini_task].Delay  -= list_task[newTaskID].Delay;
			}
			return newTaskID;
		}
		sum 	+= list_task[ini_task].Delay;
		pre_task = ini_task;
		ini_task = list_task[ini_task].NextTaskID;
	}

	//Task will add of tail
	if (ini_task == null){
		list_task[pre_task].NextTaskID = newTaskID;
		list_task[newTaskID].Delay    -= sum;
	}

	return newTaskID;
}

uint8_t SCH_Add_Task(void (*pFunc)(void), uint32_t Delay, uint32_t Period){
	return Enqueue_Task(Create_Task(pFunc, Delay, Period));
}

uint8_t SCH_Dispatch_Tasks(void){
	if (head_taskID == null || list_task[head_taskID].Delay > 0) return 0;

	//Remove head task out of queue (dequeue, not delete)
	//and reconfigure `next` and `previous pointer`.
	uint8_t runningTask = head_taskID;
	head_taskID = list_task[head_taskID].NextTaskID;

	list_task[runningTask].NextTaskID = null;
	list_task[runningTask].Delay 	  = list_task[runningTask].Period;

	//Run task
	list_task[runningTask].pFunc();
	TaskIdJustRun = runningTask;

	//Add again if a task has period value isn't equal to 0.
	if (list_task[runningTask].Period != 0) Enqueue_Task(runningTask);
	else SCH_Delete_Task(runningTask);

	return 1;
}

uint8_t SCH_Delete_Task(uint8_t TaskID){
	if (list_task[TaskID].pFunc == 0){
		errorCode = ERROR_SCH_DELETE_NULL_TASK;
		return 0;
	}
	count_task--;
	if (TaskID == head_taskID){
		head_taskID = list_task[TaskID].NextTaskID;
	}

	uint8_t next_TaskID = list_task[TaskID].NextTaskID;
	if (next_TaskID != null){
		list_task[next_TaskID].Delay    += list_task[TaskID].Delay;
	}

	uint8_t prev = 0;
	uint8_t ini = head_taskID;
	while (ini != TaskID){
		prev = ini;
		ini = list_task[ini].NextTaskID;
	}

	if (prev != null){
		list_task[prev].NextTaskID = list_task[TaskID].NextTaskID;
	}

	list_task[TaskID].pFunc 	 = nullptr;
	list_task[TaskID].NextTaskID = null;

	return 1;
}

void SCH_Sleep(void){
	//Still task need to run
	if (list_task[head_taskID].Delay == 0) return
	//No task to run at this time, then go sleep
	HAL_SuspendTick();
	HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
	HAL_ResumeTick();
}

uint8_t SCH_Report_Status(void){
#ifdef SCH_REPORT_ERRORS
	uint8_t tempErrorCode = errorCode;
	errorCode = 0;
	return tempErrorCode;
#endif
}
