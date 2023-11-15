/*
 * scheduler.c
 *
 *  Created on: Oct 29, 2023
 *      Author: HP
 */

#include "scheduler.h"

#define TIMER_CYCLE 10

struct {
	uint8_t List[MAX_TASK + 1];
	uint16_t Head;
	uint16_t Tail;
	uint8_t  Count;
	void (*pFunc_MoveHead) (void);
	void (*pFunc_MoveTail) (void);
} Vacant_ID;

void moveHead(void){
	Vacant_ID.Head++;
	if (Vacant_ID.Head > 0xFF) Vacant_ID.Head = 1;
}
void moveTail(void){
	Vacant_ID.Tail++;
	if (Vacant_ID.Tail > 0xFF) Vacant_ID.Tail = 1;
}
uint8_t isRunning(void){
	return Vacant_ID.Count < 255;
}

short head_taskID;
sTask list_task[MAX_TASK + 1]; //Zero index is not used.
uint16_t time_skip;

uint8_t errorPort;
uint8_t errorCode;
uint8_t lastErrorCode;
uint32_t errorCount;

void SCH_Init(void){
	head_taskID = null;
	time_skip = 0;

	Vacant_ID.Head = 1;
	Vacant_ID.Tail = 1;
	Vacant_ID.Count = 255;
	Vacant_ID.pFunc_MoveHead = moveHead;
	Vacant_ID.pFunc_MoveTail = moveTail;

	errorCode = 0;
	lastErrorCode = 0;
	errorCount = 30000 / TIMER_CYCLE;

	for (short i = 0; i < MAX_TASK + 1; i++){
		list_task[i].pFunc      = nullptr;
		list_task[i].NextTaskID = null;
		list_task[i].PreTaskID  = null;
		Vacant_ID.List[i] 		= i;
	}
}

void SCH_Update(void){
	if (head_taskID == null) {
		time_skip = (isRunning()) ? time_skip + 1 : 0;
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
	if (Vacant_ID.Count <= 0) return -1;
	Vacant_ID.Count--;
	uint8_t empty_idx = Vacant_ID.Head;
	Vacant_ID.pFunc_MoveHead();

	Delay  /= TIMER_CYCLE;
	Period /= TIMER_CYCLE;

	list_task[empty_idx].pFunc		  = pFunc;
	list_task[empty_idx].Delay 		  = Delay + time_skip;
	list_task[empty_idx].Period	      = Period;
	list_task[head_taskID].PreTaskID  = null;
	list_task[head_taskID].NextTaskID = null;

	return empty_idx;
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

	int diff_delay;

	while (ini_task != null){
		diff_delay = (sum + list_task[ini_task].Delay) - list_task[newTaskID].Delay;

		if (diff_delay >= 0){
			if (ini_task == head_taskID){
				list_task[newTaskID].NextTaskID = head_taskID;
				list_task[newTaskID].PreTaskID  = null;
				head_taskID = newTaskID;
				list_task[ini_task].Delay = diff_delay;
			} else {
				list_task[newTaskID].NextTaskID = ini_task;
				list_task[newTaskID].PreTaskID  = list_task[ini_task].PreTaskID;
				list_task[list_task[newTaskID].PreTaskID].NextTaskID = newTaskID;
				list_task[list_task[newTaskID].NextTaskID].PreTaskID = newTaskID;
				list_task[newTaskID].Delay -= sum;
				list_task[ini_task].Delay  -= list_task[newTaskID].Delay;
			}
		}

		sum 	+= list_task[ini_task].Delay;
		pre_task = ini_task;
		ini_task = list_task[ini_task].NextTaskID;
	}

	//Task will add of tail
	if (ini_task == null){
		list_task[pre_task].NextTaskID = newTaskID;
		list_task[newTaskID].PreTaskID = pre_task;
		list_task[newTaskID].Delay    -= sum;
	}

	return newTaskID;
}

uint8_t SCH_Add_Task(void (*pFunc)(void), uint32_t Delay, uint32_t Period){
	return Enqueue_Task(Create_Task(pFunc, Delay, Period));
}

void SCH_Dispatch_Tasks(void){
	if (head_taskID == null || list_task[head_taskID].Delay > 0) return;;

	//Remove head task out of queue (dequeue, not delete)
	//and reconfigure `next` and `previous pointer`.
	uint8_t runningTask = head_taskID;
	head_taskID = list_task[head_taskID].NextTaskID;
	if (head_taskID != null) {
		list_task[head_taskID].PreTaskID = null;
	}
	list_task[runningTask].NextTaskID = null;
	list_task[runningTask].PreTaskID  = null;
	list_task[runningTask].Delay 	 = list_task[runningTask].Period;

	//Run task
	list_task[runningTask].pFunc();

	//Add again if a task has period value isn't equal to 0.
	if (list_task[runningTask].Period != 0) Enqueue_Task(runningTask);
	else SCH_Delete_Task(runningTask);
}

uint8_t SCH_Delete_Task(uint8_t TaskID){
	if (list_task[TaskID].pFunc == 0) return -1;
	if (TaskID == head_taskID){
		head_taskID = list_task[TaskID].NextTaskID;
	}

	uint8_t next_TaskID = list_task[TaskID].NextTaskID;
	uint8_t pre_TaskID  = list_task[TaskID].PreTaskID;
	if (next_TaskID != null){
		list_task[next_TaskID].PreTaskID = list_task[TaskID].PreTaskID;
		list_task[next_TaskID].Delay    += list_task[TaskID].Delay;
	}
	if (pre_TaskID != null){
		list_task[pre_TaskID].NextTaskID = list_task[TaskID].NextTaskID;
	}
	list_task[TaskID].pFunc 	 = nullptr;
	list_task[TaskID].NextTaskID = null;
	list_task[TaskID].PreTaskID  = null;
	Vacant_ID.Tail = TaskID;
	Vacant_ID.Count += 1;
	Vacant_ID.pFunc_MoveTail();
	return 0;
}

void SCH_Report_Status(void){
#ifdef SCH_REPORT_ERRORS
	if (errorCode != lastErrorCode){
		//Negative logic on LEDs assumes
		errorPort = 255 - errorCode;
		lastErrorCode = errorCode;
		errorCount = (errorCode) ? (30000 / TIMER_CYCLE) : (0);
	} else {
		if (errorCode){
			if (errorCount){
				errorCode = 0;
			} else {
				errorCount--;
			}
		}
	}
#endif
}
