#pragma once
#include <windows.h>

namespace timers {
	class highResTimer {
	private://private variable

		//starting tickes
		LARGE_INTEGER _start;

		//ending tickes
		LARGE_INTEGER _end;

		//elapse tickes
		LARGE_INTEGER _elapse;

		//tick frequency
		LARGE_INTEGER _frequency;

		//temp
		LARGE_INTEGER _Temp;

		//running indecator
		bool _running;

	public://public variable

	public://constructor / destructor

		 /*constructor*/
		highResTimer();

		/*destructor*/
		~highResTimer();

	private://private functions

	public://public functions

		/*start the timer*/
		void start();

		/*stop the timer*/
		void stop();

		/*convert the recorded time into nano second*/
		double getNanoSecond();

		/*convert the recorded time into micro second*/
		double getMicroSecond();

		/*convert the recorded time into mili second*/
		double getMiliSecond();

		/*convert the recorded time into second*/
		double getSecond();
	};
}