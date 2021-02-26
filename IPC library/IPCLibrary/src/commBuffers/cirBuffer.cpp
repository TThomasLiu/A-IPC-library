#include "cirBuffer.h"

cirBuffer::cirBuffer(unsigned int bufferSize, unsigned int timeout, unsigned int busyLimit)
{
	_timeout = timeout;
	_busyLimit = busyLimit;
	_bufferSize = bufferSize;

	_remainLen.store(_bufferSize);
	
	_createBuffer();

	_readHead = _buffer;
	_writeHead = _buffer;
	_endHead = _buffer+_bufferSize;

	_status = statusConst::initialized;
}

cirBuffer::cirBuffer()
{
}

cirBuffer::~cirBuffer()
{
	if (_status != statusConst::notInitialized) {
		_deleteBuffer();
	}
}

void cirBuffer::_createBuffer()
{
	_buffer = (char*)malloc(_bufferSize);
}

void cirBuffer::_deleteBuffer()
{
	delete[] _buffer;
}

void cirBuffer::_reset()
{
	memset(_buffer, 0, _bufferSize);
	_writeHead = _buffer;
	_readHead = _buffer;
	_remainLen.store(_bufferSize);
}

responseConst cirBuffer::_waitForSpace(unsigned int dataSize)
{
	unsigned int start = clock();
	unsigned int dur = clock() - start;
	
	unsigned int timeout = _timeout;
	unsigned int busyLimit = _busyLimit;
	
	if (_remainLen.load() > dataSize) {
		return responseConst::success;
	}


	while (_timeout == INFINITE || dur < timeout) {
		if (_remainLen.load() >= dataSize) {
			return responseConst::success;
		}

		if (dur > busyLimit) {
			Sleep(1);
		}
		dur = clock() - start;
	}

	return responseConst::timeout;
}

responseConst cirBuffer::_waitForSpace(unsigned int dataSize, unsigned int timeoutOverride, unsigned int busyLimitOverride)
{
	unsigned int start = clock();
	unsigned int dur = clock() - start;

	unsigned int timeout = timeoutOverride;
	unsigned int busyLimit = busyLimitOverride;

	if (_remainLen.load() > dataSize) {
		return responseConst::success;
	}


	while (_timeout == INFINITE || dur < timeout) {
		if (_remainLen.load() >= dataSize) {
			return responseConst::success;
		}

		if (dur > busyLimit) {
			Sleep(1);
		}
		dur = clock() - start;
	}

	return responseConst::timeout;
}

responseConst cirBuffer::_waitForData()
{
	unsigned int start = clock();
	unsigned int dur = clock() - start;
	
	unsigned int timeout = _timeout;
	unsigned int busyLimit = _busyLimit;
	
	if (_remainLen.load() < _bufferSize) {
		return responseConst::success;
	}


	while (_timeout == INFINITE || dur < timeout) {
		if (_remainLen.load() < _bufferSize) {
			return responseConst::success;
		}

		if (dur > busyLimit) {
			Sleep(1);
		}
		
		dur = clock() - start;
	}

	return responseConst::timeout;
}

responseConst cirBuffer::_waitForData(unsigned int timeoutOverride, unsigned int busyLimitOverride)
{
	unsigned int start = clock();
	unsigned int dur = clock() - start;

	unsigned int timeout = timeoutOverride;
	unsigned int busyLimit = busyLimitOverride;

	if (_remainLen.load() < _bufferSize) {
		return responseConst::success;
	}


	while (_timeout == INFINITE || dur < timeout) {
		if (_remainLen.load() < _bufferSize) {
			return responseConst::success;
		}

		if (dur > busyLimit) {
			Sleep(1);
		}

		dur = clock() - start;
	}

	return responseConst::timeout;
}

responseConst cirBuffer::push(void* data, unsigned int dataSize)
{
	//variables and initialize
	dataHeader header;
	responseConst res;

	volatile char* currBuff = _writeHead;
	char* currRead;

	unsigned int dataFrameSize = dataSize + sizeof(dataHeader);
	unsigned int tailLen;

	//check for space
	if (_remainLen.load() < dataFrameSize) {
		res = _waitForSpace(dataFrameSize);

		if (res != responseConst::success) {
			return res;
		}
	}

	//push header
	tailLen = _endHead - _writeHead;
	header.dataLen = dataSize;
	if (tailLen < sizeof(dataHeader)) {
		currRead = (char*)&header;
		memcpy((void*)currBuff, currRead, tailLen);
		currRead += tailLen;
		memcpy(_buffer, currRead, sizeof(dataHeader) - tailLen);
		currBuff = _buffer + sizeof(dataHeader) - tailLen;
		tailLen = _endHead - currBuff;
	}
	else {
		memcpy((void*)currBuff, &header, sizeof(dataHeader));
		tailLen -= sizeof(dataHeader);
		currBuff += sizeof(dataHeader);
	}

	//push data
	if (tailLen < dataSize) {
		currRead = (char*)data;
		memcpy((void*)currBuff, currRead, tailLen);
		currRead += tailLen;
		memcpy((void*)_buffer, currRead, dataSize - tailLen);
		currBuff = _buffer + dataSize - tailLen;
	}
	else {
		memcpy((void*)currBuff, data, dataSize);
		currBuff += dataSize;
	}

	//status update
	_writeHead = currBuff;
	_remainLen -= dataFrameSize;
	return responseConst::success;

}

responseConst cirBuffer::push(void* data, unsigned int dataSize, unsigned int timeoutOverride, unsigned int busyLimitOverride)
{
	//variables and initialize
	dataHeader header;
	responseConst res;

	volatile char* currBuff = _writeHead;
	char* currRead;

	unsigned int dataFrameSize = dataSize + sizeof(dataHeader);
	unsigned int tailLen;

	//check for space
	if (_remainLen.load() < dataFrameSize) {
		res = _waitForSpace(dataFrameSize, timeoutOverride, busyLimitOverride);

		if (res != responseConst::success) {
			return res;
		}
	}

	//push header
	tailLen = _endHead - _writeHead;
	header.dataLen = dataSize;
	if (tailLen < sizeof(dataHeader)) {
		currRead = (char*)&header;
		memcpy((void*)currBuff, currRead, tailLen);
		currRead += tailLen;
		memcpy(_buffer, currRead, sizeof(dataHeader) - tailLen);
		currBuff = _buffer + sizeof(dataHeader) - tailLen;
		tailLen = _endHead - currBuff;
	}
	else {
		memcpy((void*)currBuff, &header, sizeof(dataHeader));
		tailLen -= sizeof(dataHeader);
		currBuff += sizeof(dataHeader);
	}

	//push data
	if (tailLen < dataSize) {
		currRead = (char*)data;
		memcpy((void*)currBuff, currRead, tailLen);
		currRead += tailLen;
		memcpy((void*)_buffer, currRead, dataSize - tailLen);
		currBuff = _buffer + dataSize - tailLen;
	}
	else {
		memcpy((void*)currBuff, data, dataSize);
		currBuff += dataSize;
	}

	//status update
	_writeHead = currBuff;
	_remainLen -= dataFrameSize;
	return responseConst::success;
}

responseConst cirBuffer::fetch(void** data, unsigned int* fetchSize)
{
	//variables and initialize
	volatile char* currBuff;
	char* currWrite;
	dataHeader header;
	responseConst res;
	unsigned int tailLen;
	unsigned int dataFrameSize;


	//buffer check
	if (_remainLen.load() == _bufferSize) {
		res = _waitForData();
		if (res != responseConst::success) {
			return res;
		}
	}
	tailLen = _endHead - _readHead;
	currBuff = _readHead;

	//fetch header
	if (tailLen < sizeof(dataHeader)) {
		currWrite = (char*)&header;
		memcpy(currWrite, (void*)currBuff, tailLen);
		currWrite += tailLen;
		memcpy(currWrite, (void*)_buffer, sizeof(dataHeader) - tailLen);
		currBuff = _buffer + sizeof(dataHeader) - tailLen;
		tailLen = _endHead - currBuff;
	}
	else {
		memcpy(&header, (void*)currBuff, sizeof(dataHeader));
		currBuff += sizeof(dataHeader);
		tailLen -= sizeof(dataHeader);
	}

	//header check
	if (header.startSign != 0xa5a5a5a5) {
		_reset();
		return responseConst::corrupt;
	}

	//cache preparation
	*data = malloc(header.dataLen);
	*fetchSize = header.dataLen;
	dataFrameSize = header.dataLen + sizeof(dataHeader);

	//fetch data
	if (tailLen < header.dataLen) {
		currWrite = (char*)*data;
		memcpy(currWrite, (void*)currBuff, tailLen);
		currWrite += tailLen;
		memcpy(currWrite, (void*)_buffer, header.dataLen - tailLen);
		currBuff = _buffer + header.dataLen - tailLen;
	}
	else {
		memcpy(*data, (void*)currBuff, header.dataLen);
		currBuff += header.dataLen;
	}

	//status update
	if (_readHead == _endHead) {
		*_buffer = 'A';
	}
	else {
		*_readHead = 'A';
	}
	_readHead = currBuff;
	_remainLen += dataFrameSize;

	//return
	return responseConst::success;
}

responseConst cirBuffer::fetch(void* data, int dataSize, unsigned int* fetchSize)
{
	//variables and initialize
	dataHeader header;
	responseConst res;

	unsigned int extractSize;

	unsigned int tailLen;

	volatile char* currBuff = _readHead;
	char* currWrite;

	//buffer check
	if (_remainLen.load() == _bufferSize) {
		res = _waitForData();
		if (res != responseConst::success) {
			return res;
		}
	}

	//extract header
	tailLen = _endHead - _readHead;
	if (tailLen < sizeof(dataHeader)) {
		currWrite = (char*)&header;
		memcpy(currWrite, (void*)currBuff, tailLen);
		currWrite += tailLen;
		memcpy(currWrite, (void*)_buffer, sizeof(dataHeader) - tailLen);
		currBuff = _buffer + sizeof(dataHeader) - tailLen;
		tailLen = _endHead - currBuff;
	}
	else {
		memcpy(&header, (void*)currBuff, sizeof(dataHeader));
		currBuff += sizeof(dataHeader);
		tailLen -= sizeof(dataHeader);
	}

	//header varify
	if (header.startSign != 0xa5a5a5a5) {
		_reset();
		return responseConst::corrupt;
	}

	//data cache varify
	if (header.dataLen > dataSize) {
		*fetchSize = 0;
		return responseConst::overflow;
	}

	//extract data
	if (tailLen < header.dataLen) {
		currWrite = (char*)data;
		memcpy(currWrite, (void*)currBuff, tailLen);
		currWrite += tailLen;
		memcpy(currWrite, (void*)_buffer, header.dataLen - tailLen);
		currBuff = _buffer + header.dataLen - tailLen;
	}
	else {
		memcpy(data, (void*)currBuff, header.dataLen);
		currBuff += header.dataLen;
	}

	//status update
	if (_readHead == _endHead) {
		*_buffer = 'A';
	}
	else {
		*_readHead = 'A';
	}
	_readHead = currBuff;
	_remainLen += header.dataLen + sizeof(dataHeader);
	
	//return
	*fetchSize = header.dataLen;
	return responseConst::success;
}

responseConst cirBuffer::fetch(void** data, unsigned int* fetchSize, unsigned int timeoutOverride, unsigned int busyLimitOverride)
{
	//variables and initialize
	volatile char* currBuff;
	char* currWrite;
	dataHeader header;
	responseConst res;
	unsigned int tailLen;
	unsigned int dataFrameSize;


	//buffer check
	if (_remainLen.load() == _bufferSize) {
		res = _waitForData(timeoutOverride, busyLimitOverride);
		if (res != responseConst::success) {
			return res;
		}
	}
	tailLen = _endHead - _readHead;
	currBuff = _readHead;

	//fetch header
	if (tailLen < sizeof(dataHeader)) {
		currWrite = (char*)&header;
		memcpy(currWrite, (void*)currBuff, tailLen);
		currWrite += tailLen;
		memcpy(currWrite, (void*)_buffer, sizeof(dataHeader) - tailLen);
		currBuff = _buffer + sizeof(dataHeader) - tailLen;
		tailLen = _endHead - currBuff;
	}
	else {
		memcpy(&header, (void*)currBuff, sizeof(dataHeader));
		currBuff += sizeof(dataHeader);
		tailLen -= sizeof(dataHeader);
	}

	//header check
	if (header.startSign != 0xa5a5a5a5) {
		_reset();
		return responseConst::corrupt;
	}

	//cache preparation
	*data = malloc(header.dataLen);
	*fetchSize = header.dataLen;
	dataFrameSize = header.dataLen + sizeof(dataHeader);

	//fetch data
	if (tailLen < header.dataLen) {
		currWrite = (char*)*data;
		memcpy(currWrite, (void*)currBuff, tailLen);
		currWrite += tailLen;
		memcpy(currWrite, (void*)_buffer, header.dataLen - tailLen);
		currBuff = _buffer + header.dataLen - tailLen;
	}
	else {
		memcpy(*data, (void*)currBuff, header.dataLen);
		currBuff += header.dataLen;
	}

	//status update
	if (_readHead == _endHead) {
		*_buffer = 'A';
	}
	else {
		*_readHead = 'A';
	}
	_readHead = currBuff;
	_remainLen += dataFrameSize;

	//return
	return responseConst::success;
}

responseConst cirBuffer::fetch(void* data, int dataSize, unsigned int* fetchSize, unsigned int timeoutOverride, unsigned int busyLimitOverride)
{
	//variables and initialize
	dataHeader header;
	responseConst res;

	unsigned int extractSize;

	unsigned int tailLen;

	volatile char* currBuff = _readHead;
	char* currWrite;

	//buffer check
	if (_remainLen.load() == _bufferSize) {
		res = _waitForData(timeoutOverride, busyLimitOverride);
		if (res != responseConst::success) {
			return res;
		}
	}

	//extract header
	tailLen = _endHead - _readHead;
	if (tailLen < sizeof(dataHeader)) {
		currWrite = (char*)&header;
		memcpy(currWrite, (void*)currBuff, tailLen);
		currWrite += tailLen;
		memcpy(currWrite, (void*)_buffer, sizeof(dataHeader) - tailLen);
		currBuff = _buffer + sizeof(dataHeader) - tailLen;
		tailLen = _endHead - currBuff;
	}
	else {
		memcpy(&header, (void*)currBuff, sizeof(dataHeader));
		currBuff += sizeof(dataHeader);
		tailLen -= sizeof(dataHeader);
	}

	//header varify
	if (header.startSign != 0xa5a5a5a5) {
		_reset();
		return responseConst::corrupt;
	}

	//data cache varify
	if (header.dataLen > dataSize) {
		*fetchSize = 0;
		return responseConst::overflow;
	}

	//extract data
	if (tailLen < header.dataLen) {
		currWrite = (char*)data;
		memcpy(currWrite, (void*)currBuff, tailLen);
		currWrite += tailLen;
		memcpy(currWrite, (void*)_buffer, header.dataLen - tailLen);
		currBuff = _buffer + header.dataLen - tailLen;
	}
	else {
		memcpy(data, (void*)currBuff, header.dataLen);
		currBuff += header.dataLen;
	}

	//status update
	if (_readHead == _endHead) {
		*_buffer = 'A';
	}
	else {
		*_readHead = 'A';
	}
	_readHead = currBuff;
	_remainLen += header.dataLen + sizeof(dataHeader);

	//return
	*fetchSize = header.dataLen;
	return responseConst::success;
}

responseConst cirBuffer::peek(void** data, unsigned int* fetchSize)
{
	//variables and initialize
	volatile char* currBuff;
	char* currWrite;
	dataHeader header;
	responseConst res;
	unsigned int tailLen;
	unsigned int dataFrameSize;


	//buffer check
	if (_remainLen.load() == _bufferSize) {
		res = _waitForData();
		if (res != responseConst::success) {
			return res;
		}
	}
	tailLen = _endHead - _readHead;
	currBuff = _readHead;

	//fetch header
	if (tailLen < sizeof(dataHeader)) {
		currWrite = (char*)&header;
		memcpy(currWrite, (void*)currBuff, tailLen);
		currWrite += tailLen;
		memcpy(currWrite, (void*)_buffer, sizeof(dataHeader) - tailLen);
		currBuff = _buffer + sizeof(dataHeader) - tailLen;
		tailLen = _endHead - currBuff;
	}
	else {
		memcpy(&header, (void*)currBuff, sizeof(dataHeader));
		currBuff += sizeof(dataHeader);
		tailLen -= sizeof(dataHeader);
	}

	//header check
	if (header.startSign != 0xa5a5a5a5) {
		_reset();
		return responseConst::corrupt;
	}

	//cache preparation
	*data = malloc(header.dataLen);
	*fetchSize = header.dataLen;
	dataFrameSize = header.dataLen + sizeof(dataHeader);

	//fetch data
	if (tailLen < header.dataLen) {
		currWrite = (char*)*data;
		memcpy(currWrite, (void*)currBuff, tailLen);
		currWrite += tailLen;
		memcpy(currWrite, (void*)_buffer, header.dataLen - tailLen);
		currBuff = _buffer + header.dataLen - tailLen;
	}
	else {
		memcpy(*data, (void*)currBuff, header.dataLen);
		currBuff += header.dataLen;
	}

	//return
	return responseConst::success;
}

responseConst cirBuffer::peek(void* data, int dataSize, unsigned int* fetchSize)
{
	//variables and initialize
	dataHeader header;
	responseConst res;

	unsigned int extractSize;

	unsigned int tailLen;

	volatile char* currBuff = _readHead;
	char* currWrite;

	//buffer check
	if (_remainLen.load() == _bufferSize) {
		res = _waitForData();
		if (res != responseConst::success) {
			return res;
		}
	}

	//extract header
	tailLen = _endHead - _readHead;
	if (tailLen < sizeof(dataHeader)) {
		currWrite = (char*)&header;
		memcpy(currWrite, (void*)currBuff, tailLen);
		currWrite += tailLen;
		memcpy(currWrite, (void*)_buffer, sizeof(dataHeader) - tailLen);
		currBuff = _buffer + sizeof(dataHeader) - tailLen;
		tailLen = _endHead - currBuff;
	}
	else {
		memcpy(&header, (void*)currBuff, sizeof(dataHeader));
		currBuff += sizeof(dataHeader);
		tailLen -= sizeof(dataHeader);
	}

	//header varify
	if (header.startSign != 0xa5a5a5a5) {
		_reset();
		return responseConst::corrupt;
	}

	//data cache varify
	if (header.dataLen > dataSize) {
		*fetchSize = 0;
		return responseConst::overflow;
	}

	//extract data
	if (tailLen < header.dataLen) {
		currWrite = (char*)data;
		memcpy(currWrite, (void*)currBuff, tailLen);
		currWrite += tailLen;
		memcpy(currWrite, (void*)_buffer, header.dataLen - tailLen);
		currBuff = _buffer + header.dataLen - tailLen;
	}
	else {
		memcpy(data, (void*)currBuff, header.dataLen);
		currBuff += header.dataLen;
	}

	//return
	*fetchSize = header.dataLen;
	return responseConst::success;
}

responseConst cirBuffer::peek(void** data, unsigned int* fetchSize, unsigned int timeoutOverride, unsigned int busyLimitOverride)
{
	//variables and initialize
	volatile char* currBuff;
	char* currWrite;
	dataHeader header;
	responseConst res;
	unsigned int tailLen;
	unsigned int dataFrameSize;


	//buffer check
	if (_remainLen.load() == _bufferSize) {
		res = _waitForData(timeoutOverride, busyLimitOverride);
		if (res != responseConst::success) {
			return res;
		}
	}
	tailLen = _endHead - _readHead;
	currBuff = _readHead;

	//fetch header
	if (tailLen < sizeof(dataHeader)) {
		currWrite = (char*)&header;
		memcpy(currWrite, (void*)currBuff, tailLen);
		currWrite += tailLen;
		memcpy(currWrite, (void*)_buffer, sizeof(dataHeader) - tailLen);
		currBuff = _buffer + sizeof(dataHeader) - tailLen;
		tailLen = _endHead - currBuff;
	}
	else {
		memcpy(&header, (void*)currBuff, sizeof(dataHeader));
		currBuff += sizeof(dataHeader);
		tailLen -= sizeof(dataHeader);
	}

	//header check
	if (header.startSign != 0xa5a5a5a5) {
		_reset();
		return responseConst::corrupt;
	}

	//cache preparation
	*data = malloc(header.dataLen);
	*fetchSize = header.dataLen;
	dataFrameSize = header.dataLen + sizeof(dataHeader);

	//fetch data
	if (tailLen < header.dataLen) {
		currWrite = (char*)*data;
		memcpy(currWrite, (void*)currBuff, tailLen);
		currWrite += tailLen;
		memcpy(currWrite, (void*)_buffer, header.dataLen - tailLen);
		currBuff = _buffer + header.dataLen - tailLen;
	}
	else {
		memcpy(*data, (void*)currBuff, header.dataLen);
		currBuff += header.dataLen;
	}

	//return
	return responseConst::success;
}

responseConst cirBuffer::peek(void* data, int dataSize, unsigned int* fetchSize, unsigned int timeoutOverride, unsigned int busyLimitOverride)
{
	//variables and initialize
	dataHeader header;
	responseConst res;

	unsigned int extractSize;

	unsigned int tailLen;

	volatile char* currBuff = _readHead;
	char* currWrite;

	//buffer check
	if (_remainLen.load() == _bufferSize) {
		res = _waitForData(timeoutOverride, busyLimitOverride);
		if (res != responseConst::success) {
			return res;
		}
	}

	//extract header
	tailLen = _endHead - _readHead;
	if (tailLen < sizeof(dataHeader)) {
		currWrite = (char*)&header;
		memcpy(currWrite, (void*)currBuff, tailLen);
		currWrite += tailLen;
		memcpy(currWrite, (void*)_buffer, sizeof(dataHeader) - tailLen);
		currBuff = _buffer + sizeof(dataHeader) - tailLen;
		tailLen = _endHead - currBuff;
	}
	else {
		memcpy(&header, (void*)currBuff, sizeof(dataHeader));
		currBuff += sizeof(dataHeader);
		tailLen -= sizeof(dataHeader);
	}

	//header varify
	if (header.startSign != 0xa5a5a5a5) {
		_reset();
		return responseConst::corrupt;
	}

	//data cache varify
	if (header.dataLen > dataSize) {
		*fetchSize = 0;
		return responseConst::overflow;
	}

	//extract data
	if (tailLen < header.dataLen) {
		currWrite = (char*)data;
		memcpy(currWrite, (void*)currBuff, tailLen);
		currWrite += tailLen;
		memcpy(currWrite, (void*)_buffer, header.dataLen - tailLen);
		currBuff = _buffer + header.dataLen - tailLen;
	}
	else {
		memcpy(data, (void*)currBuff, header.dataLen);
		currBuff += header.dataLen;
	}

	//return
	*fetchSize = header.dataLen;
	return responseConst::success;
}
