#include <stdlib.h>
#include "threadsafe.h"

#if defined(_MSC_VER)

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

#undef near
#undef far

#endif

/*-------------------------------------------------------------------------------------------------
* Timer Implementation
*------------------------------------------------------------------------------------------------*/

#if defined(_WIN32)

static unsigned int ClockMicroseconds()
{
	LARGE_INTEGER frequency, counter;	
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&counter);

	if (frequency.QuadPart >= 1000000)
	{
		return (unsigned int)(counter.QuadPart / (frequency.QuadPart / 1000000));
	}
	else
	{
		return (unsigned int)(counter.QuadPart * (1000000 / frequency.QuadPart));
	}
}

#else

static unsigned int ClockMicroseconds()
{
	timespec tp;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tp);
	return (unsigned int)(tp.tv_sec * 1000000 + tp.tv_nsec / 1000);
}

#endif /* _WIN32 */

/*-------------------------------------------------------------------------------------------------
* Atomic Implementation
*------------------------------------------------------------------------------------------------*/

#if defined(_MSC_VER)

int AtomicLoadInteger(volatile int* destination)
{
	return *destination;
}

void AtomicStoreInteger(volatile int* destination, int value)
{
	*destination = value;
}

int AtomicAddInteger(volatile int* destination, int value)
{
	return InterlockedAdd((LONG*)destination, value);
}

int AtomicSwapInteger(volatile int* destination, int value)
{
	return InterlockedExchange((LONG*)destination, value);
}

int AtomicCompareAndSwapInteger(volatile int* destination, int value, int comparand)
{
	return InterlockedCompareExchange((LONG*)destination, value, comparand);
}

#elif defined(COMPILER_GCC)



#elif defined(COMPILER_CLANG)



#endif

/*-------------------------------------------------------------------------------------------------
* Spinlock Implementation
*------------------------------------------------------------------------------------------------*/

int TryLockSpinLock(int* destination)
{
	return AtomicSwapInteger(destination, 1) == 0;
}

void LockSpinLock(int* destination)
{
	while (AtomicSwapInteger(destination, 1) == 1)
		while (AtomicLoadInteger(destination) == 1);
}

void UnlockSpinLock(int* destination)
{
	AtomicStoreInteger(destination, 0);
}

/*-------------------------------------------------------------------------------------------------
* Futex Implementation
*------------------------------------------------------------------------------------------------*/

#if defined(_WIN32)

#pragma comment(lib, "Synchronization") /* WaitOnAddress, WakeByAddressSingle, WakeByAddressAll */

void FutexWait(int* destination, int comparand)
{
	do
	{
		WaitOnAddress(destination, &comparand, sizeof(int), INFINITE);
	} while (AtomicLoadInteger(destination) == comparand);
}

void FutexWakeSingle(int* destination)
{
	WakeByAddressSingle(destination);
}

void FutexWakeAll(int* destination)
{
	WakeByAddressAll(destination);
}

#endif /* _WIN32 */

/*-------------------------------------------------------------------------------------------------
* Thread Implementation
*------------------------------------------------------------------------------------------------*/

#if defined(_WIN32)

typedef struct
{
	void (*function)(void* data);
	void* data;
} Win32ThreadWrapperData;

static DWORD Win32ThreadWrapper(void* data)
{
	Win32ThreadWrapperData* wrapperData = (Win32ThreadWrapperData*)data;
	wrapperData->function(wrapperData->data);
	return 0;
}

Thread InitThread(void(*function)(void* data), void* data)
{
	Win32ThreadWrapperData* wrapperData = malloc(sizeof(Win32ThreadWrapperData));
	wrapperData->function = function;
	wrapperData->data = data;

	return CreateThread(NULL, 0, &Win32ThreadWrapper, wrapperData, 0, NULL);
}

void JoinThread(Thread thread)
{
	WaitForSingleObject(thread, INFINITE);
	CloseHandle(thread);
}

void DetachThread(Thread thread)
{
	CloseHandle(thread);
}

#endif /* _WIN32 */

/*-------------------------------------------------------------------------------------------------
* Task System Implementation
*------------------------------------------------------------------------------------------------*/

enum TaskSystemTaskFlags
{
	TASK_SYSTEM_TASK_FLAG_QUIT = 1
};

/* Declare static functions */
static int TaskListIsEmpty(TaskList* list);
static int TryPopWork(TaskList* list);
static int TryPopWorkSingleConsumer(TaskList* list);
static int PushWork(TaskList* list, int index);
static void PushDone(TaskSystemWorker* worker, int index);
static int TryPopFree(TaskSystemWorker* worker);
static int PopFree(TaskSystemWorker* worker);
static int WorkerTryHelp(TaskSystemWorker* worker, int allowSleep);
static void WorkerSleep(TaskSystemWorker* worker);
static void WakeUpWorker(TaskSystemWorker* worker);
static void FinishTask(TaskSystemWorker* worker, int index);
static void ExecuteTask(TaskSystemWorker* worker, int index);

static int TaskListIsEmpty(TaskList* list)
{
	return AtomicLoadInteger(&list->first) == -1;
}

static int TryPopWork(TaskList* list)
{
	int tmp, result, first = AtomicLoadInteger(&list->first);

	if (first == -1 && AtomicCompareAndSwapInteger(&list->first, first, first) == -1)
		return -1;

	while (1)
	{
		if (first == -1)
		{
			return -1;
		}
		else if (first == -2)
		{
			while ((first = AtomicLoadInteger(&list->first)) == -2);

			if (first == -1)
				return -1;
		}

		tmp = AtomicLoadInteger(&list->nodes[first].nextWork);

		if (tmp != -1)
		{
			result = AtomicCompareAndSwapInteger(&list->first, tmp, first);
			if (result == first)
				return first;

			first = result;
			continue;
		}
		else
		{
			result = AtomicCompareAndSwapInteger(&list->first, -2, first);
			if (result != first)
			{
				first = result;
				continue;
			}

			int last = first;
			if (AtomicCompareAndSwapInteger(&list->last, -1, last) == last)
			{
				AtomicStoreInteger(&list->first, -1);
				return first;
			}
			else
			{
				while ((tmp = AtomicLoadInteger(&list->nodes[first].nextWork)) == -1);

				AtomicStoreInteger(&list->first, tmp);
				return first;
			}
		}
	}
}

static int TryPopWorkSingleConsumer(TaskList* list)
{
	int tmp, first = AtomicLoadInteger(&list->first);

	while (1)
	{
		if (first == -1)
			return -1;

		tmp = AtomicLoadInteger(&list->nodes[first].nextWork);

		if (tmp != -1)
		{
			AtomicStoreInteger(&list->first, tmp);
			return first;
		}
		else
		{
			int last = first;
			if (AtomicCompareAndSwapInteger(&list->last, tmp, last) == last)
			{
				AtomicStoreInteger(&list->first, tmp);
				return first;
			}
			else
			{
				while ((tmp = AtomicLoadInteger(&list->nodes[first].nextWork)) == -1);

				AtomicStoreInteger(&list->first, tmp);
				return first;
			}
		}
	}
}

static int PushWork(TaskList* list, int index)
{
	AtomicStoreInteger(&list->nodes[index].nextWork, -1);

	int last = AtomicSwapInteger(&list->last, index);

	if (last != -1)
	{
		AtomicStoreInteger(&list->nodes[last].nextWork, index);
		return 0;
	}

	while (AtomicCompareAndSwapInteger(&list->first, index, -1) != -1)
		while (AtomicLoadInteger(&list->first) != -1);

	return 1;
}

static void PushDone(TaskSystemWorker* worker, int index)
{
	if (worker->doneListSize == worker->blockSize)
	{
		worker->workList.nodes[worker->doneListEnd].nextFree = -1;
		PushWork(&worker->taskSystem->freeList, worker->doneListStart);
	}
	else if (worker->doneListSize > 0)
	{
		worker->workList.nodes[worker->doneListEnd].nextFree = index;
		worker->doneListEnd = index;
		worker->doneListSize++;
		return;
	}

	worker->doneListStart = index;
	worker->doneListEnd = index;
	worker->doneListSize = 1;
}

static int TryPopFree(TaskSystemWorker* worker)
{
	TaskNode* nodes = worker->workList.nodes;
	int tmp;

	// pop from worker free list
	if (worker->freeListStart != -1)
	{
		tmp = worker->freeListStart;
		worker->freeListStart = nodes[tmp].nextFree;
		return tmp;
	}

	// pop from task system free list
	tmp = TryPopWork(&worker->taskSystem->freeList);

	if (tmp != -1)
	{
		worker->freeListStart = nodes[tmp].nextFree;
		return tmp;
	}

	return -1;
}

static int PopFree(TaskSystemWorker* worker)
{
	while (1)
	{
		int tmp = TryPopFree(worker);

		if (tmp != -1)
			return tmp;

		tmp = TryPopWork(&worker->workList);

		if (tmp != -1)
		{
			ExecuteTask(worker, tmp);
			continue;
		}

		(void)WorkerTryHelp(worker, 0);
	}
}

static int WorkerTryHelp(TaskSystemWorker* worker, int allowSleep)
{
	TaskSystem* taskSystem = worker->taskSystem;
	TaskSystemHelper* helper = &taskSystem->helper;
	TaskSystemWorker* workers;

	int myThreadIndex;
	int task;
	int workerCount;

	if (!TryLockSpinLock(&helper->lock))
		return 0;

	workers = taskSystem->workers;
	myThreadIndex = worker->index - 1;
	workerCount = taskSystem->coreCount - 1;

	if (helper->pushIndex == myThreadIndex)
		helper->pushIndex = (helper->pushIndex + 1) % workerCount;


	/* Split work from helper queue. */

	while (1)
	{
		TaskSystemWorker* currentWorker = workers + 1 + helper->pushIndex;
		task = TryPopWorkSingleConsumer(&helper->list);

		if (task == -1)
			break;

		if (PushWork(&currentWorker->workList, task))
			WakeUpWorker(currentWorker);

		helper->pushIndex = (helper->pushIndex + 1) % workerCount;

		if (helper->pushIndex != myThreadIndex)
			continue;

		helper->pushIndex = (helper->pushIndex + 1) % workerCount;
		continue;
	}


	/* Maybe split work from workers to reduce stealing overhead. */

	/* Steal work */

	if (myThreadIndex == -1)
	{
		for (int i = 0; i < workerCount; i++)
		{
			task = TryPopWork(&workers[1 + i].workList);

			if (task != -1)
			{
				PushWork(&worker->workList, task);
				UnlockSpinLock(&helper->lock);
				return 1;
			}
		}
	}
	else
	{
		for (int i = 0; i < myThreadIndex; i++)
		{
			task = TryPopWork(&workers[1 + i].workList);

			if (task != -1)
			{
				PushWork(&worker->workList, task);
				UnlockSpinLock(&helper->lock);
				return 1;
			}
		}

		for (int i = myThreadIndex + 1; i < workerCount; i++)
		{
			task = TryPopWork(&workers[1 + i].workList);

			if (task != -1)
			{
				PushWork(&worker->workList, task);
				UnlockSpinLock(&helper->lock);
				return 1;
			}
		}
	}

	/* Sleep */

	if (!allowSleep)
	{
		UnlockSpinLock(&helper->lock);
		return 1;
	}

	int tmp = AtomicLoadInteger(&helper->state);
	while (1)
	{
		int target = (tmp >= 1 && tmp <= 2) ? tmp - 1 : tmp;
		int result = AtomicCompareAndSwapInteger(&helper->state, target, tmp);

		if (result == tmp)
		{
			if (tmp >= 0 && tmp <= 1)
				FutexWait(&helper->state, 0);

			break;
		}

		tmp = result;
	}

	UnlockSpinLock(&helper->lock);
	return 1;
}

static void WorkerSleep(TaskSystemWorker* worker)
{
	TaskSystem* taskSystem = worker->taskSystem;
	TaskSystemHelper* helper = &taskSystem->helper;
	int tmp;

	while (1)
	{
		if (WorkerTryHelp(worker, 1))
			return;

		if (AtomicLoadInteger(&helper->state) == 0)
			break;

		unsigned int start = ClockMicroseconds();

		while (ClockMicroseconds() - start < 100)
		{
			if (!TaskListIsEmpty(&worker->workList))
				return;
		}
	}

	tmp = AtomicLoadInteger(&worker->state);

	while (1)
	{
		int value = tmp >= 1 && tmp <= 2 ? tmp - 1 : tmp;
		int result = AtomicCompareAndSwapInteger(&worker->state, value, tmp);

		if (result == tmp)
		{
			if (tmp >= 0 && tmp <= 1)
			{
				if (WorkerTryHelp(worker, 1))
					return;

				FutexWait(&worker->state, 0);
			}

			break;
		}

		tmp = result;
	}
}

static void WakeUpWorker(TaskSystemWorker* worker)
{
	int tmp = AtomicLoadInteger(&worker->state);
	while (1)
	{
		int value = tmp >= 0 && tmp <= 1 ? tmp + 1 : tmp;
		int result = AtomicCompareAndSwapInteger(&worker->state, value, tmp);

		if (result == tmp)
		{
			if (tmp == 0)
				FutexWakeSingle(&worker->state);

			break;
		}

		tmp = result;
	}
}

static void FinishTask(TaskSystemWorker* worker, int index)
{
	TaskNode* node = worker->workList.nodes + index;
	int count = AtomicAddInteger(&node->childCount, -1);

	if (count == 0)
	{
		if (node->parent.index != -1)
			FinishTask(worker, node->parent.index);

		PushDone(worker, index);
	}
}

static void ExecuteTask(TaskSystemWorker* worker, int index)
{
	TaskNode* nodes = worker->workList.nodes;
	TaskNode* task = nodes + index;

	if (task->dependency.index != -1 && AtomicLoadInteger(&nodes[task->dependency.index].generation)
		== task->dependency.generation)
	{
		TaskHandle handle = { index, nodes[index].generation };
		SubmitTask(worker, handle);
		return;
	}

	if (task->function != NULL)
		task->function(worker, task->args);

	FinishTask(worker, index);
}

static void TaskSystemWorkerLoop(void* data)
{
	TaskSystemWorker* worker = (TaskSystemWorker*)data;

	while (1)
	{
		int task;

		while (1)
		{
			task = TryPopWork(&worker->workList);

			if (task != -1)
				break;

			WorkerSleep(worker);
		}

		if (worker->workList.nodes[task].flags == TASK_SYSTEM_TASK_FLAG_QUIT)
			break;

		ExecuteTask(worker, task);
	}
}

/* nodeCount must be a multiple of blockSize. */
static void InitTaskSystemFreeList(int nodeCount, int blockSize, TaskList* nodeList)
{
	TaskNode* nodes = nodeList->nodes;

	for (int i = 0; i < nodeCount; i++)
	{
		if ((i % blockSize) == 0)
		{
			int next = i + blockSize;

			if (i != 0)
			{
				nodes[i - 1].nextFree = -1;
			}

			nodes[i].nextWork = nodeCount > next ? next : -1;
		}

		nodes[i].generation = 0;
		nodes[i].nextFree = i + 1;
	}

	nodes[nodeCount - 1].nextFree = -1;
	nodeList->first = 0;
	nodeList->last = nodeCount - blockSize;
}

TaskSystem* InitTaskSystem(int threadCount, TaskSystemWorker** mainWorker)
{
	int blockSize;
	int blockCount;
	int nodeCount;
	TaskList* nodeList;
	TaskSystemWorker* workers;
	TaskSystemWorker defaultWorker;
	Thread* threads;
	TaskSystem* taskSystem;
	TaskSystemHelper* helper;

	if (threadCount == 0)
		return NULL;

	taskSystem = (TaskSystem*)malloc(sizeof(TaskSystem));

	if (taskSystem == NULL)
		return NULL;

	blockSize = 256;
	blockCount = threadCount * 3;
	nodeCount = blockSize * blockCount;

	nodeList = &taskSystem->freeList;
	nodeList->nodes = (TaskNode*)malloc(sizeof(TaskNode) * nodeCount);

	if (nodeList->nodes == NULL)
		goto cleanTaskSystem;

	taskSystem->nodeCount = nodeCount;
	taskSystem->blockSize = blockSize;
	InitTaskSystemFreeList(nodeCount, blockSize, nodeList);

	helper = &taskSystem->helper;
	helper->lock = 0;
	helper->state = 1;
	helper->pushIndex = 0;
	helper->list.first = -1;
	helper->list.last = -1;
	helper->list.nodes = nodeList->nodes;

	workers = (TaskSystemWorker*)malloc(sizeof(TaskSystemWorker) * threadCount);

	if (workers == NULL)
		goto cleanNodes;

	taskSystem->workers = workers;
	taskSystem->coreCount = threadCount;

	defaultWorker.taskSystem = taskSystem;
	defaultWorker.doneListStart = -1;
	defaultWorker.doneListEnd = -1;
	defaultWorker.doneListSize = 0;
	defaultWorker.blockSize = blockSize;
	defaultWorker.workList.first = -1;
	defaultWorker.workList.last = -1;
	defaultWorker.workList.nodes = nodeList->nodes;
	defaultWorker.state = 0;

	for (int i = 0; i < threadCount; i++)
	{
		int node = TryPopWorkSingleConsumer(nodeList);
		TaskSystemWorker* worker = workers + i;

		if (node == -1)
			goto cleanWorkers;

		*worker = defaultWorker;
		worker->freeListStart = node;
		worker->index = i;
	}

	threads = (Thread*)malloc(sizeof(Thread) * (threadCount - 1));

	if (threads == NULL)
		goto cleanWorkers;

	taskSystem->threads = threads;

	for (int i = 1; i < threadCount; i++)
	{
		threads[i - 1] = InitThread(&TaskSystemWorkerLoop, workers + i);
	}

	if (mainWorker != NULL)
		*mainWorker = taskSystem->workers;

	return taskSystem;

cleanWorkers:
	free(workers);

cleanNodes:
	free(nodeList->nodes);

cleanTaskSystem:
	free(taskSystem);
	return NULL;
}

void CloseTaskSystem(TaskSystem* taskSystem)
{
	for (int i = 0; i < taskSystem->coreCount; i++)
	{
		TaskHandle emptyTask = { -1, -1 };
		TaskHandle quitTask = InitTask(taskSystem->workers, NULL, NULL, emptyTask, emptyTask);
		taskSystem->freeList.nodes[quitTask.index].flags = TASK_SYSTEM_TASK_FLAG_QUIT;
		SubmitTask(taskSystem->workers, quitTask);
	}

	for (int i = 0, end = taskSystem->coreCount - 1; i < end; i++)
	{
		JoinThread(taskSystem->threads[i]);
	}

	free(taskSystem->threads);
	free(taskSystem->workers);
	free(taskSystem);
}

TaskHandle InitTask(TaskSystemWorker* worker, TaskFunction function, void* args, TaskHandle
	dependency, TaskHandle parent)
{
	TaskHandle handle = { -1, -1 };
	int index = PopFree(worker);

	TaskNode* task = worker->workList.nodes + index;
	task->function = function;
	task->args = args;
	task->dependency = dependency;
	task->parent = parent;
	task->flags = 0;
	task->childCount = 1;

	if (parent.index != -1)
	{
		TaskNode* parentNode = worker->workList.nodes + parent.index;
		parentNode->childCount++;
	}

	handle.index = index;
	handle.generation = AtomicLoadInteger(&task->generation);

	return handle;
}

void SubmitTask(TaskSystemWorker* worker, TaskHandle task)
{
	TaskSystem* taskSystem = worker->taskSystem;
	TaskSystemHelper* helper = &taskSystem->helper;

	if (PushWork(&helper->list, task.index))
	{
		int tmp = AtomicLoadInteger(&helper->state);
		while (1)
		{
			int value = tmp >= 0 && tmp <= 1 ? tmp + 1 : tmp;
			int result = AtomicCompareAndSwapInteger(&helper->state, value, tmp);

			if (result == tmp)
			{
				if (tmp == 0)
					FutexWakeSingle(&helper->state);

				break;
			}

			tmp = result;
		}
	}
}

void WaitOnTask(TaskSystemWorker* worker, TaskHandle task)
{
	while (1)
	{
		int tmp;

		if (AtomicLoadInteger(&worker->workList.nodes[task.index].generation) != task.generation)
			return;

		tmp = TryPopWork(&worker->workList);

		if (tmp != -1)
		{
			ExecuteTask(worker, tmp);
			continue;
		}

		(void)WorkerTryHelp(worker, 0);
	}
}
