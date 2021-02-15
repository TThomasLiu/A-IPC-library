#include "multithread.h"

static DWORD WINAPI threadStarter(LPVOID param) {
	multithreadFuncWrapper* package = (multithreadFuncWrapper*)param;

	//*(package->_status) = statusConst::running;
	package->func(package->param);
	*(package->_status) = statusConst::halt;

	return 0;
}

threadManager::threadManager()
{
	_status = statusConst::notInitialized;
	_thread = INVALID_HANDLE_VALUE;
	_cacheSize = 0;
	_paramCache = NULL;
}

threadManager::~threadManager()
{
	_releaseCache();
}

void threadManager::_releaseCache()
{
	if (_status != statusConst::notInitialized) {
		_status = statusConst::notInitialized;
		delete[] _paramCache;
		_cacheSize = 0;
	}
}

responseConst threadManager::loadThreadParameter(multithreadFuncPtr func, void* data, int dataSize)
{
	//status check
	if (_status == statusConst::running) {
		return responseConst::failed;
	}

	//release cache data
	_releaseCache();

	//copy data into cache
	_paramCache = (char*)malloc(dataSize);
	memcpy(_paramCache, data, dataSize);

	//build wrapper
	_wrapper.func = func;
	_wrapper.param = _paramCache;
	_wrapper._status = &_status;

	//return and status update
	_status = statusConst::initialized;
	return responseConst::success;
}

responseConst threadManager::startThread()
{
	if (_status == statusConst::notInitialized) {
		return responseConst::failed;
	}

	if (_status == statusConst::running) {
		return responseConst::failed;
	}

	_status = statusConst::running;
	_thread = CreateThread(NULL, 0, threadStarter, &_wrapper, 0, (LPDWORD)&_threadId);
	return responseConst::success;
}

void threadManager::joinThread()
{
	if (_status != statusConst::running) {
		return;
	}

	WaitForSingleObject(_thread, INFINITE);
}

void threadManager::killThread()
{
	if (_status == statusConst::running) {
		TerminateThread(_thread, NULL);
		_status = statusConst::halt;
	}
}
