#include <iostream>
#include <string.h>
#include "IPCLibrary.h"
#include "src/timer/timer.h"

#define testRound 1000000

struct speedTestPacket {
	char payload[128];
};

void buffOpSendSubthreadPrint(void* param) {
	char scrBuff[1001];
	unsigned int fetchSize;
	bufferOperator<cirBuffer>* buff = *(bufferOperator<cirBuffer>**)param;
	while (1) {
		buff->fetch(scrBuff, 1000, &fetchSize, INFINITE, 10);
		scrBuff[fetchSize] = 0;
		buff->push(scrBuff, fetchSize);
		std::cout << scrBuff << std::endl;
	}

}

void buffOpSendSubthreadCtrl(void* param) {
	timers::highResTimer timerTotal;
	timers::highResTimer timerSend;
	timers::highResTimer timerRecv;
	char scrBuff[1001];
	char read[1001];
	unsigned int fetchSize;
	bufferOperator<cirBuffer>* buff = *(bufferOperator<cirBuffer>**)param;
	while (1) {
		std::cin >> read;
		timerTotal.start();
		timerSend.start();
		buff->push(read, strlen(read));
		timerSend.stop();
		timerRecv.start();
		buff->fetch(scrBuff, 1000, &fetchSize, INFINITE, 10);
		timerRecv.stop();
		timerTotal.stop();
		scrBuff[fetchSize] = 0;
		std::cout << "echoed : " << scrBuff << std::endl;
		std::cout << "msg round trip cost " << timerTotal.getMicroSecond() << "us" << std::endl;
		std::cout << "msg send cost " << timerSend.getMicroSecond() << "us" << std::endl;
		std::cout << "msg recv cost " << timerRecv.getMicroSecond() << "us" << std::endl;
	}

}

void buffOpMultithreadTest() {
	threadManager scr;
	threadManager read;
	bufferPair<cirBuffer> bp(10000, 1000, 10);
	bufferOperator<cirBuffer>* screr;
	bufferOperator<cirBuffer>* reader;
	
	screr = bp.getBuffer();
	reader = bp.getBuffer();

	scr.loadThreadParameter(buffOpSendSubthreadPrint, &screr, sizeof(bufferOperator<cirBuffer>*));
	read.loadThreadParameter(buffOpSendSubthreadCtrl, &reader, sizeof(bufferOperator<cirBuffer>*));
	scr.startThread();
	read.startThread();
	scr.joinThread();
	read.joinThread();
}

void bufferOpTest() {
	int send = 0;
	int recv;
	unsigned int recvSize;
	bufferPair<cirBuffer> bp(1000, 100, 10);
	bufferOperator<cirBuffer>* a;
	bufferOperator<cirBuffer>* b;

	responseConst res;

	a = bp.getBuffer();
	b = bp.getBuffer();
	for (int i = 0; i < 100000; i++) {
		res = a->push(&i, sizeof(int));
		//std::cout << res << " a push: " << i << std::endl;
		res = b->fetch(&recv, sizeof(int), &recvSize);
		//std::cout << res << " b recv: " << recv << std::endl;


		res = b->push(&i, sizeof(int));
		//std::cout << res << " b push: " << i << std::endl;
		res = a->fetch(&recv, sizeof(int), &recvSize);
		//std::cout << res << " a recv: " << recv << std::endl;

	}

}

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
	speedTestPacket* recv2;
	int recv;
	unsigned int recvSize;
	long long int totalRecvSize = 0;

	timer.start();
	for (int i = 0; i < testRound; i++) {
		res = buffer->fetch(&packet, sizeof(speedTestPacket), &recvSize);
		//res = buffer->fetch((void**)&recv2, &recvSize);
		if (res != responseConst::success) {
			std::cout << "recv res: " << res << std::endl;
		}
		totalRecvSize += recvSize;
		//delete recv2;
	}

	timer.stop();

	printf("recv speed:%f MBytes/sec\n", totalRecvSize / timer.getSecond() / 1000000);
	printf("recv %f msg/sec\n", testRound / timer.getSecond());
	std::cout << "recv testRound in:" << timer.getMicroSecond() << "us" << std::endl;
	std::cout << "recv sub thread end\n";
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
	//	std::cout << "send: " << send << "res: " << res << std::endl;;
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
	//bufferOpTest();
	//serialTest();
	buffOpMultithreadTest();
	std::cout << "end" << std::endl;
	return 0;
}