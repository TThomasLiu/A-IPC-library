#include <iostream>
#include "IPCLibrary.h"
#include "src/timer/timer.h"

#define testRound 10000000

struct speedTestPacket {
	char payload[128];
};

void testSubThread(void* param) {
	int* a = *(int**)param;
	std::cout << *a << std::endl;
	Sleep(1000);
	std::cout << *a + 1 << std::endl;
}

void sendSubThread(void* param) {
	responseConst res;
	cirBuffer* buffer = *(cirBuffer**)param;
	timers::highResTimer timer;
	speedTestPacket packet;
	long long int sendSize = 0;
	timer.start();
	for (int i = 0; i < testRound; i++) {
		res = buffer->push(&packet, sizeof(speedTestPacket));
		if (res != responseConst::success) {
			std::cout << "send res: " << res << std::endl;
		}
		else {
			sendSize += sizeof(speedTestPacket);
		}

	}
	timer.stop();
	printf("send speed:%f MBytes/sec\n", sendSize / timer.getSecond() / 1000000);
	printf("send%f msg/sec\n", testRound / timer.getSecond());
	std::cout << "send testRound in:" << timer.getMicroSecond() << "us" << std::endl;
	std::cout << "send sub thread end\n";
}

void recvSubThread(void* param) {
	cirBuffer* buffer = *(cirBuffer**)param;
	responseConst res;
	timers::highResTimer timer;
	speedTestPacket packet;
	int recv;
	unsigned int recvSize;
	long long int totalRecvSize = 0;

	timer.start();
	for (int i = 0; i < testRound; i++) {
		res = buffer->fetch(&packet, sizeof(speedTestPacket), &recvSize);
		if (res != responseConst::success) {
			std::cout << "recv res: " << res << std::endl;
		}
		totalRecvSize += recvSize;
	}

	timer.stop();

	printf("recv speed:%f MBytes/sec\n", totalRecvSize / timer.getSecond() / 1000000);
	printf("recv %f msg/sec\n", testRound / timer.getSecond());
	std::cout << "recv testRound in:" << timer.getMicroSecond() << "us" << std::endl;
	std::cout << "recv sub thread end\n";
}

void serialTest() {

}

void threadManagerTest() {
	threadManager t;
	timers::highResTimer timer;
	int a = 10;
	int* b = &a;
	responseConst res;
	
	timer.start();
	t.loadThreadParameter(testSubThread, &b, sizeof(b));
	timer.stop();
	std::cout << timer.getMicroSecond() << "us" << std::endl;

	timer.start();
	res = t.startThread();
	timer.stop();
	std::cout << timer.getMicroSecond() << "us" << std::endl;
	//std::cout << t.startThread() << std::endl;
	t.joinThread();
}

void cirBuffTest() {
	responseConst res;
	cirBuffer t(12* 100, 100, 100);
	int send = -1;
	int recv = -1;
	int* recv2;
	unsigned int fetchSize;


	for (int i = 0; i < 100; i++) {
		send = i;
		res = t.push(&send, sizeof(int));
		std::cout << "send: " << send << "res: " << res << std::endl;;
	}

	for (int i = 0; i < 100; i++) {
		res = t.fetch(&recv, sizeof(int), &fetchSize);
		std::cout << "recv: " << recv << "res: " << res << std::endl;
	}

	for (int i = 0; i < 100; i++) {
		send = i;
		res = t.push(&send, sizeof(int));
		std::cout << "send: " << send << "res: " << res << std::endl;;
	}

	for (int i = 0; i < 100; i++) {
		res = t.fetch((void**)&recv2, &fetchSize);
		std::cout << "recv: " << *recv2 << "res: " << res << std::endl;
		delete recv2;
	}
}

void cirBuffMultithreadTest() {
	threadManager send;
	threadManager recv;
	cirBuffer buff(1000000, 1000, 100);
	cirBuffer* buffPtr = &buff;

	send.loadThreadParameter(sendSubThread, &buffPtr, sizeof(cirBuffer*));
	recv.loadThreadParameter(recvSubThread, &buffPtr, sizeof(cirBuffer*));
	send.startThread();
	recv.startThread();
	send.joinThread();
	recv.joinThread();
}

int main() {
	timers::highResTimer timer;
	speedTestPacket a;
	speedTestPacket b;
	long long int totalSize = 0;
	std::cout << "start" << std::endl;
	//threadManagerTest();
	//for(int i=0;i<5;i++)
	//cirBuffTest();
	//cirBuffMultithreadTest();
	
	serialTest();
	std::cout << "end" << std::endl;
	return 0;
}