#pragma once
#include <Windows.h>
#include <time.h>
#include "..\utilitiesConstants.h"

using multithreadFuncPtr = void(*)(void*);

struct multithreadFuncWrapper {
	volatile statusConst* _status;
	multithreadFuncPtr func;
	void* param;
};

class threadManager {
private://private variables
	multithreadFuncWrapper _wrapper;
	statusConst _status;
	HANDLE _thread;
	
	char* _paramCache;
	int _cacheSize;
	int _threadId;

public://public variables

public://con / destructor
	threadManager();
	~threadManager();

private://private functions
	void _releaseCache();

public://public functions
	responseConst loadThreadParameter(multithreadFuncPtr func, void* data, int dataSize);
	responseConst startThread();
	void joinThread();
	void killThread();
};