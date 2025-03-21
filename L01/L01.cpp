#include <windows.h>
#include <iostream>
#include "SmirnovSession.h"
#include "SmirnovThread.h"
using namespace std;

typedef bool (*ReadData_t)(char*, size_t);

void start()
{
    HMODULE hDll = LoadLibraryA("MMF.dll");

    ReadData_t ReadData = (ReadData_t)GetProcAddress(hDll, "ReadData");
	
	InitializeCriticalSection(&cs);

	vector<SmirnovThread*> threads;
    boolean flag = true;

    HANDLE hStartEvent = CreateEvent(NULL, FALSE, FALSE, L"StartEvent");
    HANDLE hStopEvent = CreateEvent(NULL, FALSE, FALSE, L"StopEvent");
    HANDLE hSendEvent = CreateEvent(NULL, FALSE, FALSE, L"SendEvent");
    HANDLE hConfirmEvent = CreateEvent(NULL, FALSE, FALSE, L"ConfirmEvent");
    HANDLE hControlEvents[3] = {hStartEvent, hStopEvent, hSendEvent};
    while (flag)
    {
        int n = WaitForMultipleObjects(3, hControlEvents, FALSE, INFINITE) - WAIT_OBJECT_0;
        
        switch (n)
        {
        case 0: {
            
            auto thread = new SmirnovThread();
            threads.push_back(thread);
            SetEvent(hConfirmEvent);
            break;
        }
        case 1: {
            ResetEvent(hStopEvent);
            if (!threads.empty()) {
                threads.back()->addMessage(MT_CLOSE);
                delete threads.back();
                threads.pop_back();
            }
            else {
                flag = false;
            }
            SetEvent(hConfirmEvent);
            break;
        }
        case 2: {
            ResetEvent(hSendEvent);
            char buffer[4096];
            ReadData(buffer, sizeof(buffer));
            if (!threads.empty()) {
                threads.back()->addMessage(MT_DATA, buffer);
            }
            SetEvent(hConfirmEvent);
            break;
        }
        }
    }
    SetEvent(hConfirmEvent);
	DeleteCriticalSection(&cs);
}

int main()
{
	start();
	return 0;
}
