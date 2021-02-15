#include "timer.h"

timers::highResTimer::highResTimer()
{
	_start.QuadPart = 0;
	_end.QuadPart = 0;
	_elapse.QuadPart = 0;
	_frequency.QuadPart = 0;
	_Temp.QuadPart = 0;
	_running = false;
}

timers::highResTimer::~highResTimer()
{
}

void timers::highResTimer::start()
{
	_running = true;
	QueryPerformanceFrequency(&_frequency);
	QueryPerformanceCounter(&_start);
}

void timers::highResTimer::stop()
{
	QueryPerformanceCounter(&_end);
	if (!_running) {
		_elapse.QuadPart = -1;
		return;
	}
	_elapse.QuadPart = _end.QuadPart - _start.QuadPart;
}

double timers::highResTimer::getNanoSecond()
{
	if (_elapse.QuadPart == -1) {
		return -1.0;
	}

	_Temp.QuadPart = _elapse.QuadPart;
	_Temp.QuadPart *= 1000000000;//10^9
	return((double)_Temp.QuadPart) / ((double)_frequency.QuadPart);
}

double timers::highResTimer::getMicroSecond()
{
	if (_elapse.QuadPart == -1) {
		return -1.0;
	}

	_Temp.QuadPart = _elapse.QuadPart;
	_Temp.QuadPart *= 1000000;//10^6
	return((double)_Temp.QuadPart) / ((double)_frequency.QuadPart);
}

double timers::highResTimer::getMiliSecond()
{
	if (_elapse.QuadPart == -1) {
		return -1.0;
	}

	_Temp.QuadPart = _elapse.QuadPart;
	_Temp.QuadPart *= 1000;//10^3
	return((double)_Temp.QuadPart) / ((double)_frequency.QuadPart);
}

double timers::highResTimer::getSecond()
{
	if (_elapse.QuadPart == -1) {
		return -1.0;
	}

	_Temp.QuadPart = _elapse.QuadPart;
	return((double)_Temp.QuadPart) / ((double)_frequency.QuadPart);
}
