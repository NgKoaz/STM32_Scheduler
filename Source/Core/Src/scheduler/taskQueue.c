/*
 * taskQueue.c
 *
 *  Created on: Nov 29, 2024
 *      Author: HP
 */


#include <scheduler/taskQueue.h>

#define MINIMUM_ARRAY_LENGTH SCH_MAX_TASKS + 1

sTask tasks[MINIMUM_ARRAY_LENGTH];		// 0 is null pointer.
uint16_t count;						// Number of tasks are running.
uint16_t timeSkip;					// The error of time.
MY_TYPE headTaskIdQueue;
MY_TYPE tailTaskIdQueue;


IdleID idleIdQueue[MINIMUM_ARRAY_LENGTH];
MY_TYPE headIdleIdQueue;
MY_TYPE tailIdleIdQueue;


void initQueue(void) {
	headTaskIdQueue = null;
	tailTaskIdQueue = null;
	timeSkip = 0;
	count = 0;

	for (uint16_t i = 1; i < MINIMUM_ARRAY_LENGTH; i++) {
		idleIdQueue[i].nextId = i + 1;
		if (i == MINIMUM_ARRAY_LENGTH)
			idleIdQueue[i].nextId = null;
	}
	headIdleIdQueue = 1;
	tailIdleIdQueue = MINIMUM_ARRAY_LENGTH - 1;

}

uint8_t _dequeueIdleId() {
	IdleID idleId = idleIdQueue[headIdleIdQueue];
	if (headIdleIdQueue != null) {
		uint8_t newId = headIdleIdQueue;
		headIdleIdQueue = idleId.nextId;
		idleId.nextId = null;
		if (headIdleIdQueue == null) {
			tailIdleIdQueue = null;
		}
		return newId;
	}
	return null;
}


MY_TYPE _enqueueTask(MY_TYPE newTaskId) {
	//First task in queue.
	if (headTaskIdQueue == null){
		headTaskIdQueue = newTaskId;
		tailTaskIdQueue = newTaskId;
		return newTaskId;
	}

	//Task will add at middle of queue.
	MY_TYPE curTaskId = headTaskIdQueue;
	uint32_t sum = 0;
	while (curTaskId != null){
		if (sum + tasks[curTaskId].delay > tasks[newTaskId].delay){
			if (curTaskId == headTaskIdQueue){
				tasks[newTaskId].nextId = headTaskIdQueue;
				headTaskIdQueue = newTaskId;
				tasks[curTaskId].delay -= tasks[newTaskId].delay;
				tasks[curTaskId].previousId = headTaskIdQueue;
			} else {
				tasks[newTaskId].nextId = curTaskId;
				MY_TYPE oldPreId = tasks[curTaskId].previousId;

				tasks[newTaskId].previousId = oldPreId;
				tasks[curTaskId].previousId = newTaskId;

				tasks[oldPreId].nextId = newTaskId;

				tasks[newTaskId].delay -= sum;
				tasks[curTaskId].delay  -= tasks[newTaskId].delay;
			}
			return newTaskId;
		}
		sum 	+= tasks[curTaskId].delay;
		curTaskId = tasks[curTaskId].nextId;
	}

	//Task will add of tail
	if (curTaskId == null) {
		tasks[tailTaskIdQueue].nextId = newTaskId;
		tasks[newTaskId].previousId = tailTaskIdQueue;
		tailTaskIdQueue = newTaskId;
		tasks[newTaskId].delay    -= sum;
	}

	return newTaskId;
}


MY_TYPE createTask(void (*pFunc)(void), uint32_t delay, uint32_t period) {
	MY_TYPE newId = _dequeueIdleId();
	if (newId == null) return -1;

	delay  /= TIMER_CYCLE;
	period /= TIMER_CYCLE;

	tasks[newId].pFunc		= pFunc;
	tasks[newId].delay 		= delay + timeSkip;
	tasks[newId].period	    = period;
	tasks[newId].nextId 	= null;
	tasks[newId].previousId = null;
	count++;

	_enqueueTask(newId);
	return newId;
}


uint8_t deleteTask(MY_TYPE deletedTaskId) {
	count--;

	MY_TYPE preDeletedId = tasks[deletedTaskId].previousId;
	MY_TYPE nextDeletedId = tasks[deletedTaskId].nextId;

	if (deletedTaskId == headTaskIdQueue){
		if (headTaskIdQueue == tailTaskIdQueue) {
			headTaskIdQueue = null;
			tailTaskIdQueue = null;
			tasks[deletedTaskId].nextId = null;
			tasks[deletedTaskId].previousId = null;
		} else {
			tasks[nextDeletedId].delay += tasks[deletedTaskId].delay;
			tasks[preDeletedId].nextId = nextDeletedId;
			tasks[nextDeletedId].previousId = preDeletedId;
			headTaskIdQueue = tasks[deletedTaskId].nextId;
			tasks[deletedTaskId].nextId = null;
			tasks[deletedTaskId].previousId = null;
		}
		return 1;
	}

	if (preDeletedId != null) tasks[preDeletedId].nextId = nextDeletedId;
	if (nextDeletedId != null){
		tasks[nextDeletedId].delay += tasks[deletedTaskId].delay;
		tasks[nextDeletedId].previousId = preDeletedId;
	} else {
		tailTaskIdQueue = preDeletedId;
		idleIdQueue[tailTaskIdQueue].nextId = null;
	}

	tasks[deletedTaskId].nextId = null;
	tasks[deletedTaskId].previousId = null;

	return 1;
}


uint8_t dispatchTask(void) {
	if (headTaskIdQueue == null || tasks[headTaskIdQueue].delay > 0) return 0;

	//Remove head task out of queue (dequeue, not delete)
	//and reconfigure `next` and `previous pointer`.
	uint8_t readyTaskId = headTaskIdQueue;
	headTaskIdQueue = tasks[headTaskIdQueue].nextId;
	if (readyTaskId == tailTaskIdQueue) {
		headTaskIdQueue = null;
		tailTaskIdQueue = null;
	} else {
		tasks[headTaskIdQueue].previousId = null;
	}

	tasks[readyTaskId].previousId = null;
	tasks[readyTaskId].nextId = null;
	tasks[readyTaskId].delay = tasks[readyTaskId].period;

	//Run task
	tasks[readyTaskId].pFunc();
	TaskIdJustRun = readyTaskId;

	//Add again if a task has period value isn't equal to 0.
	if (tasks[readyTaskId].period != 0) _enqueueTask(readyTaskId);
	else deleteTask(readyTaskId);

	return 1;
}

void update(void) {
	if (headTaskIdQueue == null) {
		timeSkip = (count > 0) ? timeSkip + 1 : 0;
		return;
	}
	timeSkip++;
	if (tasks[headTaskIdQueue].delay > 0){
		int temp = tasks[headTaskIdQueue].delay - timeSkip;
		if (temp >= 0) {
			tasks[headTaskIdQueue].delay = temp;
			timeSkip = 0;
		} else {
			tasks[headTaskIdQueue].delay = 0;
			timeSkip = 0 - temp;
		}
	}
}

void sleep(void) {
	//Still task need to run
	if (tasks[headTaskIdQueue].delay == 0) return
	//No task to run at this time, then go sleep
	HAL_SuspendTick();
	HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
	HAL_ResumeTick();
}

