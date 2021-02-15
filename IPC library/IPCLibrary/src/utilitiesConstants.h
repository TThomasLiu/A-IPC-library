#pragma once
enum responseConst {
	success = 0,
	failed = -1,
	timeout = -2,
	overflow = -3,
	invalidValue = -4,
	corrupt = -5
};

enum statusConst {
	initialized = 0,
	notInitialized = 1,
	running = 2,
	halt = 3,
	pause = 4
};