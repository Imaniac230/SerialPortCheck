#ifndef __SERIAL_TEST_H__
#define __SERIAL_TEST_H__

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tchar.h>
#include <conio.h>
#include <assert.h>
#include <process.h>

#ifdef __cplusplus

#include <cstdio>
#include <cstring>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

#else

#include <stdio.h>
#include <string.h>

#endif
//#define ALL_CONSOLE_OUTPUT
//#define LOG_RECEIVED_MESSAGE
#define WRITE_TIMEOUT INFINITE
#define READ_TIMEOUT INFINITE
#define DETECTION_TIMEOUT (long long)50
#define QUERECONNECT (DWORD)0

enum TError {
	ENoMemory = 0,
	};

void PrintCommState(const DCB & dcb);
void SetCommParameters(DCB & dcb);


#endif /* __SERIAL_TEST_H__ */
