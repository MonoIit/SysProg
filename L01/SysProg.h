#pragma once

#include "pch.h"

using namespace std;

inline void DoWrite()
{
	std::cout << std::endl;
}

template <class T, typename... Args> inline void DoWrite(T& value, Args... args)
{
	std::wcout << value << L" ";
	DoWrite(args...);
}

static CRITICAL_SECTION cs;
static bool initCS = true;
template <typename... Args> inline void SafeWrite(Args... args)
{
	if (initCS)
	{
		InitializeCriticalSection(&cs);
		initCS = false;
	}
	EnterCriticalSection(&cs);
	DoWrite(args...);
	LeaveCriticalSection(&cs);
}

