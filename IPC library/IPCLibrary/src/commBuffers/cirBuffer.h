#pragma once
#include <Windows.h>
#include <atomic>
#include <time.h>
#include <type_traits>

#include "..\utilitiesConstants.h"

/*circular buffer class*/
class cirBuffer {
private://private struct
	/*header of data frame	*/
	struct dataHeader {
		const int startSign = 0xa5a5a5a5;//data frame starting sign
		unsigned int dataLen;//length of data
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
	cirBuffer();
	~cirBuffer();

private://private functions
	virtual void _createBuffer();
	virtual void _deleteBuffer();
	virtual void _reset();
	
	virtual responseConst _waitForSpace(unsigned int dataSize);
	virtual responseConst _waitForSpace(unsigned int dataSize, unsigned int timeoutOverride, unsigned int busyLimitOverride);

	virtual responseConst _waitForData();
	virtual responseConst _waitForData(unsigned int timeoutOverride, unsigned int busyLimitOverride);

public://public functions
	virtual responseConst push(void* data, unsigned int dataSize);
	virtual responseConst push(void* data, unsigned int dataSize, unsigned int timeoutOverride, unsigned int busyLimitOverride);

	virtual responseConst fetch(void** data, unsigned int* fetchSize);
	virtual responseConst fetch(void* data, int dataSize,unsigned int*fetchSize);
	virtual responseConst fetch(void** data, unsigned int* fetchSize, unsigned int timeoutOverride, unsigned int busyLimitOverride);
	virtual responseConst fetch(void* data, int dataSize, unsigned int* fetchSize, unsigned int timeoutOverride, unsigned int busyLimitOverride);

	virtual responseConst peek(void** data, unsigned int* fetchSize);
	virtual responseConst peek(void* data, int dataSize, unsigned int* fetchSize);
	virtual responseConst peek(void** data, unsigned int* fetchSize, unsigned int timeoutOverride, unsigned int busyLimitOverride);
	virtual responseConst peek(void* data, int dataSize, unsigned int* fetchSize, unsigned int timeoutOverride, unsigned int busyLimitOverride);

};

template <typename cirBufferDeriveType>
class bufferOperator : cirBuffer{
private://private structs
	struct bufferBoundle {
		cirBufferDeriveType* readBuff;
		cirBufferDeriveType* writeBuff;
	};

private://private variables
	statusConst _status = statusConst::notInitialized;
	bufferBoundle _commBuffers;

public://public variables

public://con / destructor
	bufferOperator();
	bufferOperator(cirBufferDeriveType* readBuff, cirBufferDeriveType* writeBuff);
	~bufferOperator();

private://private functions
	responseConst _checkInit();

public://public functions
	responseConst _loadBuffer(cirBufferDeriveType* readBuff, cirBufferDeriveType* writeBuff);
	responseConst push(void* data, unsigned int dataSize) override;
	responseConst push(void* data, unsigned int dataSize, unsigned int timeoutOverride, unsigned int busyLimitOverride) override;
	
	responseConst fetch(void** data, unsigned int* fetchSize)override;
	responseConst fetch(void* data, int dataSize, unsigned int* fetchSize)override;
	responseConst fetch(void** data, unsigned int* fetchSize, unsigned int timeoutOverride, unsigned int busyLimitOverride) override;
	responseConst fetch(void* data, int dataSize, unsigned int* fetchSize, unsigned int timeoutOverride, unsigned int busyLimitOverride)override;
	
	responseConst peek(void** data, unsigned int* fetchSize)override;
	responseConst peek(void* data, int dataSize, unsigned int* fetchSize)override;
	responseConst peek(void** data, unsigned int* fetchSize, unsigned int timeoutOverride, unsigned int busyLimitOverride)override;
	responseConst peek(void* data, int dataSize, unsigned int* fetchSize, unsigned int timeoutOverride, unsigned int busyLimitOverride)override;

};

template <typename cirBufferDeriveType>
class bufferPair {
private://private structs
	struct bufferOperatorControl {
		volatile bool enable = true;
		bufferOperator<cirBufferDeriveType> buffOp;
	};

private://private variables
	cirBufferDeriveType* _buffA;
	cirBufferDeriveType* _buffB;

	bufferOperatorControl _buffOp[2];

	unsigned int _bufferSizeA;
	unsigned int _bufferSizeB;
	unsigned int _timeoutA;
	unsigned int _timeoutB;
	unsigned int _busyLimitA;
	unsigned int _busyLimitB;

	statusConst _status = statusConst::notInitialized;

public://public variables

public://con / destructors
	bufferPair(unsigned int bufferSize, unsigned int timeout, unsigned int busyLimit);
	bufferPair(unsigned int bufferSizeA, unsigned int bufferSizeB, unsigned int timeout, unsigned int busyLimit);
	~bufferPair();

private://private functions
	responseConst _bondBuffer();

public://public functions
	bufferOperator<cirBufferDeriveType>* getBuffer();
};


/*buffer operator template class methods*/

template<typename cirBufferDeriveType>
inline bufferOperator<cirBufferDeriveType>::bufferOperator()
{
}

template<typename cirBufferDeriveType>
inline bufferOperator<cirBufferDeriveType>::bufferOperator(cirBufferDeriveType* readBuff, cirBufferDeriveType* writeBuff)
{
	responseConst res;
	res = _loadBuffer(readBuff, writeBuff);

	if (res != responseConst::success) {
		return;
	}
}

template<typename cirBufferDeriveType>
inline bufferOperator<cirBufferDeriveType>::~bufferOperator()
{
}

template<typename cirBufferDeriveType>
inline responseConst bufferOperator<cirBufferDeriveType>::_checkInit()
{
	if (_status == statusConst::notInitialized) {
		return responseConst::failed;
	}
	return responseConst::success;
}

template<typename cirBufferDeriveType>
inline responseConst bufferOperator<cirBufferDeriveType>::_loadBuffer(cirBufferDeriveType* readBuff, cirBufferDeriveType* writeBuff)
{
	if (readBuff == NULL || writeBuff == NULL) {
		return responseConst::invalidValue;
	}

	_commBuffers.readBuff = readBuff;
	_commBuffers.writeBuff = writeBuff;
	_status = statusConst::initialized;

	return responseConst::success;
}

template<typename cirBufferDeriveType>
inline responseConst bufferOperator<cirBufferDeriveType>::push(void* data, unsigned int dataSize)
{
	responseConst res;
	res = _commBuffers.writeBuff->push(data, dataSize);
	return res;
}

template<typename cirBufferDeriveType>
inline responseConst bufferOperator<cirBufferDeriveType>::push(void* data, unsigned int dataSize, unsigned int timeoutOverride, unsigned int busyLimitOverride)
{
	responseConst res;
	res = _commBuffers.writeBuff->push(data, dataSize, timeoutOverride, busyLimitOverride);
	return res;
}

template<typename cirBufferDeriveType>
inline responseConst bufferOperator<cirBufferDeriveType>::fetch(void** data, unsigned int* fetchSize)
{
	responseConst res;
	res = _commBuffers.readBuff->fetch(data, fetchSize);
	return res;
}

template<typename cirBufferDeriveType>
inline responseConst bufferOperator<cirBufferDeriveType>::fetch(void* data, int dataSize, unsigned int* fetchSize)
{
	responseConst res;
	res = _commBuffers.readBuff->fetch(data, dataSize, fetchSize);
	return res;
}

template<typename cirBufferDeriveType>
inline responseConst bufferOperator<cirBufferDeriveType>::fetch(void** data, unsigned int* fetchSize, unsigned int timeoutOverride, unsigned int busyLimitOverride)
{
	responseConst res;
	res = _commBuffers.readBuff->fetch(data, fetchSize, timeoutOverride, busyLimitOverride);
	return res;
}

template<typename cirBufferDeriveType>
inline responseConst bufferOperator<cirBufferDeriveType>::fetch(void* data, int dataSize, unsigned int* fetchSize, unsigned int timeoutOverride, unsigned int busyLimitOverride)
{
	responseConst res;
	res = _commBuffers.readBuff->fetch(data, dataSize, fetchSize, timeoutOverride, busyLimitOverride);
	return res;
}

template<typename cirBufferDeriveType>
inline responseConst bufferOperator<cirBufferDeriveType>::peek(void** data, unsigned int* fetchSize)
{
	responseConst res;
	res = _commBuffers.readBuff->peek(data, fetchSize);
	return res;
}

template<typename cirBufferDeriveType>
inline responseConst bufferOperator<cirBufferDeriveType>::peek(void* data, int dataSize, unsigned int* fetchSize)
{
	responseConst res;
	res = _commBuffers.readBuff->peek(data, dataSize, fetchSize);
	return res;
}

template<typename cirBufferDeriveType>
inline responseConst bufferOperator<cirBufferDeriveType>::peek(void** data, unsigned int* fetchSize, unsigned int timeoutOverride, unsigned int busyLimitOverride)
{
	responseConst res;
	res = _commBuffers.readBuff->peek(data, fetchSize, timeoutOverride, busyLimitOverride);
	return res;
}

template<typename cirBufferDeriveType>
inline responseConst bufferOperator<cirBufferDeriveType>::peek(void* data, int dataSize, unsigned int* fetchSize, unsigned int timeoutOverride, unsigned int busyLimitOverride)
{
	responseConst res;
	res = _commBuffers.readBuff->peek(data, dataSize, fetchSize, timeoutOverride, busyLimitOverride);
	return res;
}

/*buffer pair template class methods*/

template<typename cirBufferDeriveType>
inline bufferPair<cirBufferDeriveType>::bufferPair(unsigned int bufferSize, unsigned int timeout, unsigned int busyLimit)
{

	responseConst res;

	_bufferSizeA = bufferSize;
	_bufferSizeB = bufferSize;
	_timeoutA = timeout;
	_timeoutB = timeout;
	_busyLimitA = busyLimit;
	_busyLimitB = busyLimit;
	
	_buffA = new cirBufferDeriveType(_bufferSizeA, _timeoutA, _busyLimitA);
	_buffB = new cirBufferDeriveType(_bufferSizeB, _timeoutB, _busyLimitB);

	_status = statusConst::initialized;
	res = _bondBuffer();

}

template<typename cirBufferDeriveType>
inline bufferPair<cirBufferDeriveType>::bufferPair(unsigned int bufferSizeA, unsigned int bufferSizeB, unsigned int timeout, unsigned int busyLimit)
{

	responseConst res;

	_bufferSizeA = bufferSizeA;
	_bufferSizeB = bufferSizeB;
	_timeoutA = timeout;
	_timeoutB = timeout;
	_busyLimitA = busyLimit;
	_busyLimitB = busyLimit;

	_buffA = new cirBufferDeriveType(_bufferSizeA, _timeoutA, _busyLimitA);
	_buffB = new cirBufferDeriveType(_bufferSizeB, _timeoutB, _busyLimitB);

	_status = statusConst::initialized;
	res = _bondBuffer();

}

template<typename cirBufferDeriveType>
inline bufferPair<cirBufferDeriveType>::~bufferPair()
{
	if (_status != statusConst::notInitialized) {
		delete _buffA;
		delete _buffB;
	}
}

template<typename cirBufferDeriveType>
inline responseConst bufferPair<cirBufferDeriveType>::_bondBuffer()
{
	responseConst resA;
	responseConst resB;
	resA = _buffOp[0].buffOp._loadBuffer(_buffA, _buffB);
	resB = _buffOp[1].buffOp._loadBuffer(_buffB, _buffA);
	
	if (resA == responseConst::success && resB == responseConst::success) {
		return responseConst::success;
	}
	return responseConst::failed;
}

template<typename cirBufferDeriveType>
inline bufferOperator<cirBufferDeriveType>* bufferPair<cirBufferDeriveType>::getBuffer()
{
	for (int i = 0; i < 2; i++) {
		if (_buffOp[i].enable) {
			_buffOp[i].enable = false;
			return &_buffOp[i].buffOp;
		}
	}
	return NULL;
}

