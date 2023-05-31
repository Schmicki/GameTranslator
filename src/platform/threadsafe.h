#pragma once

/**************************************************************************************************
*	
*	threadsafe - Everything multithreading
*	
*	FEATURES:
* 
*	- Threads
*	- Atomic operations
*	- Task System
*	
**************************************************************************************************/

/*-------------------------------------------------------------------------------------------------
* Defines
*------------------------------------------------------------------------------------------------*/

/* Define cache line size */
#define THREADSAFE_CACHE_LINE 64

#if defined(_MSC_VER)
	#define THREADSAFE_ALIGN(alignment) __declspec(align(alignment))
#else
	#define THREADSAFE_ALIGN(alignment) __attribute__((aligned(alignment)))
#endif

/*-------------------------------------------------------------------------------------------------
* Structure and Type Definitions
*------------------------------------------------------------------------------------------------*/

typedef void* Thread;

typedef void (*TaskFunction)(struct TaskSystemWorker* taskSystem, void* args);

typedef struct
{
	int index;
	int generation;
} TaskHandle;

typedef THREADSAFE_ALIGN(THREADSAFE_CACHE_LINE) struct TaskNode
{
	TaskHandle parent;
	TaskHandle dependency;
	TaskFunction function;
	void* args;
	int flags;
	int nextFree;
	int nextWork;	/* atomic */
	int generation; /* atomic */
	int childCount; /* atomic */
} TaskNode;

typedef THREADSAFE_ALIGN(THREADSAFE_CACHE_LINE) struct TaskList
{
	int first; /* atomic */
	char _pad0[THREADSAFE_CACHE_LINE - sizeof(int)];
	int last;  /* atomic */
	char _pad1[THREADSAFE_CACHE_LINE - sizeof(int)];
	TaskNode* nodes;
	char _pad5[THREADSAFE_CACHE_LINE - sizeof(int)];
} TaskList;

typedef THREADSAFE_ALIGN(THREADSAFE_CACHE_LINE) struct TaskSystemHelper
{
	int lock;		/* atomic */
	char _pad0[THREADSAFE_CACHE_LINE - sizeof(int)];
	int state;		/* atomic */
	char _pad1[THREADSAFE_CACHE_LINE - sizeof(int)];
	int pushIndex;	/* atomic */
	char _pad2[THREADSAFE_CACHE_LINE - sizeof(int)];
	TaskList list;	
} TaskSystemHelper;

typedef THREADSAFE_ALIGN(THREADSAFE_CACHE_LINE) struct TaskSystemWorker
{
	struct TaskSystem* taskSystem;
	int freeListStart;
	int doneListStart;
	int doneListEnd;
	int doneListSize;
	int blockSize;
	int index;
	TaskList workList;
	int state; /* atomic */
} TaskSystemWorker;

typedef THREADSAFE_ALIGN(THREADSAFE_CACHE_LINE) struct TaskSystem
{
	TaskList freeList;
	int nodeCount;
	int blockSize;
	int coreCount;
	char _pad0[THREADSAFE_CACHE_LINE - sizeof(int) * 3];
	TaskSystemHelper helper;
	TaskSystemWorker* workers;
	Thread* threads;
} TaskSystem;

/*-------------------------------------------------------------------------------------------------
* Function Declarations
*------------------------------------------------------------------------------------------------*/

/* Atomic Functions */
int AtomicLoadInteger(volatile int* destination);
void AtomicStoreInteger(volatile int* destination, int value);
int AtomicSwapInteger(volatile int* destination, int value);
int AtomicCompareAndSwapInteger(volatile int* destination, int value, int comparand);

/* Spinlock Functions */
int TryLockSpinLock(int* destination);
void LockSpinLock(int* destination);
void UnlockSpinLock(int* destination);

/* Futex Functions */
void FutexWait(int* destination, int comparand);
void FutexWakeSingle(int* destination);
void FutexWakeAll(int* destination);

/* Thread Functions */
Thread InitThread(void(*function)(void* data), void* data);
void JoinThread(Thread thread);
void DetachThread(Thread thread);

/* TaskSystem Functions */
TaskSystem* InitTaskSystem(int threadCount, TaskSystemWorker** mainWorker);
void CloseTaskSystem(TaskSystem* taskSystem);
TaskHandle InitTask(TaskSystemWorker* worker, TaskFunction function, void* args,
	TaskHandle dependency, TaskHandle parent);
void SubmitTask(TaskSystemWorker* worker, TaskHandle task);
void WaitOnTask(TaskSystemWorker* worker, TaskHandle task);