#include "..\IPCLibrary.h"

//spinlock
int utility::spinLock::_createLockId()
{
	int cache = _lockId;
	cache++;
	if (cache < 0)
		return -cache;
	return cache;
}

int utility::spinLock::lock(int timeout)
{
	int start = clock();
	while (timeout == INFINITE || (clock() - start) < timeout) {
		if (clock() - start > _switchDelay) {
			Sleep(_sleepTime);
		}
		if (!*_lock) {
			*_lock = true;
			return _createLockId();
		}
	}
	return -1;
}

int utility::spinLock::unlock(int lockId)
{
	if (lockId != _lockId) {
		return -1;
	}
	*_lock = false;
	return 0;
}

utility::spinLock::spinLock(volatile bool* lock) {
	//input check
	if (lock == NULL) {
		exception::error("spin lock", "lock can\'t be NULL");
	}

	//value assign
	_lock = lock;
}

utility::spinLock::spinLock(volatile bool* lock, int switchDelay, int sleepTime)
{
	//input check
	if (lock == NULL) {
		exception::error("spin lock", "lock can\'t be NULL");
	}
	if (switchDelay < 0) {
		exception::error("spin lock", "switchDelay can\'t be negative");
	}
	if (sleepTime < 0) {
		exception::error("spin lock", "sleepTime can\'t be negative");
	}
	if (sleepTime == 0) {
		exception::warning("spin lock", "sleep disabled");
	}

	//value assign
	_lock = lock;
	_switchDelay = switchDelay;
	_sleepTime = sleepTime;
}

utility::spinLock::~spinLock()
{
}

//inernal share memory
int internal::internalCommShareMem::realloc(int size)
{
	char* memCache;

	if (size < memSize) {
		return -1;
	}

	if (size == memSize) {
		return 0;
	}

	//move old content to memCache
	memCache = new char[memSize];
	memset(memCache, 0, memSize);
	memcpy(memCache, mem, memSize);
	delete[] mem;

	//allocate new block of memory and move old content back to mem
	mem = new char[size];
	memset(mem, 0, size);
	memcpy(mem, memCache, memSize);
	memSize = size;
	delete[] memCache;

	return 0;
}

internal::internalCommShareMem::internalCommShareMem(int size) {
	memSize = size;
	mem = new char[size];
	memset(mem, 0, size);
	_allocFlag = true;
}

internal::internalCommShareMem::~internalCommShareMem() {
	if (_allocFlag) {
		delete[]mem;
	}
}

