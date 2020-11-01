#pragma once
#include <iostream>
#include <time.h>
#include <Windows.h>

namespace exception{
	void error(std::string modual, std::string msg);
	void warning(std::string modual, std::string msg);
}

namespace utility{

	/*Spin lock
	***description***
	* the lock use busy waiting
	*/
	class spinLock {
	private://private variables
		
		//the delay before busy waiting loop interduce sleep function (ms)
		int _switchDelay = 5;

		//the duration of busy waiting loop sleep (ms)
		int _sleepTime = 1;

		//the id that lock the spin lock
		volatile int _lockId;

		//the lock object
		volatile bool* _lock;
	
	private://private functions

		//return a lock Id
		int _createLockId();

	public://public functions

		/*lock the spin lock
		* return value:
		* positive integer(lockId): success
		* -1: waiting timeout
		*/
		int lock(int timeout);

		/*unlock the spin lock
		* return value:
		* 0: success
		* -1: lock id mismatch
		*/
		int unlock(int lockId);

		/*check if the lock is locked
		* return value:
		* 0: success
		* -1: timeout
		*/
		int chklock(int timeout);

	public://constructor & destructor

		/*constructor
		* use default switchDelay and sleepTime
		* parameters:
		* lock: bool pointer to the boolean variable storage spin lock status
		*/
		spinLock(volatile bool* lock);

		/*constructor
		* parameters:
		* lock: bool pointer to the boolean variable storage spin lock status
		* switchDelay: the duration before busy waiting loop include sleep
		* sleepTime: the sleep duration in busy waiting loop
		*/
		spinLock(volatile bool* lock, int switchDelay, int sleepTime);

		/*destructor
		*/
		~spinLock();
	};

}

namespace internal {
	class internalCommShareMem {
	private:
		bool _allocFlag = false;
		volatile bool _memLock = false;

	public:
		//the memory block size
		int memSize;

		//pointer to memory block
		char* mem;

	public:
		/*change the memory block size
		* return value:
		* 0: success
		* -1: size error(new size < old size)
		*/
		int realloc(int size);

	public://con/destructor
		internalCommShareMem(int size);
		~internalCommShareMem();
	};
}
