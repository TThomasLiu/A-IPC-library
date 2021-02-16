#pragma once
#include <Windows.h>
#include <atomic>
#include <time.h>

#include "..\utilitiesConstants.h"

class cirBuffer {
private://private struct
	struct dataHeader {
		const int startSign = 0xa5a5a5a5;
		unsigned int dataLen;
	};

private://private variables
	statusConst _status = statusConst::notInitialized;
	
	char* _buffer;
	volatile char* _readHead;
	volatile char* _writeHead;
	char* _endHead;

	unsigned int _bufferSize;
	std::atomic<unsigned int> _remainLen;

	unsigned int _timeout;
	unsigned int _busyLimit;

public://public variables

public://con / destructor
	cirBuffer(unsigned int bufferSize, unsigned int timeout, unsigned int busyLimit);
	~cirBuffer();

private://private functions
	virtual void _createBuffer();
	virtual void _deleteBuffer();
	void _reset();
	
	responseConst _waitForSpace(unsigned int dataSize);
	responseConst _waitForSpace(unsigned int dataSize, unsigned int timeoutOverride, unsigned int busyLimitOverride);

	responseConst _waitForData();
	responseConst _waitForData(unsigned int timeoutOverride, unsigned int busyLimitOverride);

public://public functions
	responseConst push(void* data, unsigned int dataSize);
	responseConst push(void* data, unsigned int dataSize, unsigned int timeoutOverride, unsigned int busyLimitOverride);

	responseConst fetch(void** data, unsigned int* fetchSize);
	responseConst fetch(void* data, int dataSize,unsigned int*fetchSize);
	responseConst fetch(void** data, unsigned int* fetchSize, unsigned int timeoutOverride, unsigned int busyLimitOverride);
	responseConst fetch(void* data, int dataSize, unsigned int* fetchSize, unsigned int timeoutOverride, unsigned int busyLimitOverride);

	responseConst peek(void** data, unsigned int* fetchSize);
	responseConst peek(void* data, int dataSize, unsigned int* fetchSize);
	responseConst peek(void** data, unsigned int* fetchSize, unsigned int timeoutOverride, unsigned int busyLimitOverride);
	responseConst peek(void* data, int dataSize, unsigned int* fetchSize, unsigned int timeoutOverride, unsigned int busyLimitOverride);

};

