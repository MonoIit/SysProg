#include <windows.h>
#include <iostream>
#include "SmirnovSession.h"
#include "SmirnovThread.h"
using namespace std;

typedef bool (*ReadDataFunc)(int&, char*, size_t);

void start(HMODULE hDLL, ReadDataFunc ReadData)
{
    
	
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
            char buffer[4096] = { 0 };
            int threadId = -1;

            ReadData(threadId, buffer, sizeof(buffer));

            if (threadId == -1) {
                SafeWrite("Главный поток:", buffer);
            }
            else if (threadId == -2) {
                for (auto thread : threads) {
                    thread->addMessage(MT_DATA, buffer);
                }
            }
            else if (!threads.empty()) {
                threads[threadId]->addMessage(MT_DATA, buffer);
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
    HMODULE hDLL = LoadLibraryA("MMF.dll");

    ReadDataFunc ReadData = (ReadDataFunc)GetProcAddress(hDLL, "ReadData");

	start(hDLL, ReadData);
	return 0;
}
