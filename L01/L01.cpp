#include <windows.h>
#include <iostream>
#include "SmirnovSession.h"
#include "SmirnovThread.h"
using namespace std;

struct header {
    int addr;
    int size;
};

typedef wstring (*ReadDataFunc)(header& h);

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

            header h;
            wstring msg = ReadData(h);


            if (h.addr == -1) {
                SafeWrite(L"Главный поток:", msg);
            }
            else if (h.addr == -2) {
                for (auto thread : threads) {
                    thread->addMessage(MT_DATA, msg);
                }
            }
            else if (!threads.empty()) {
                threads[h.addr]->addMessage(MT_DATA, msg);
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
    SetConsoleOutputCP(CP_UTF8);
    std::locale::global(std::locale(""));
    std::wcout.imbue(std::locale(""));

    HMODULE hDLL = LoadLibraryA("MMF.dll");

    ReadDataFunc ReadData = (ReadDataFunc)GetProcAddress(hDLL, "ReadData");

	start(hDLL, ReadData);
	return 0;
}
