#include <windows.h>
#include <iostream>
#include "SmirnovSession.h"
using namespace std;


DWORD WINAPI MyThread(LPVOID lpParam)
{
	auto session = static_cast<SmirnovSession*>(lpParam);
	SafeWrite("session", session->sessionID, "created");
	while (true)
	{
		Message m;
		if (session->getMessage(m))
		{
			switch (m.header.messageType)
			{
			case MT_CLOSE:
			{
				SafeWrite("session", session->sessionID, "closed");
				delete session;
				return 0;
			}
			case MT_DATA:
			{
				SafeWrite("session", session->sessionID, "data", m.data);
				Sleep(500 * session->sessionID);
				break;
			}
			}
		}
	}
	return 0;
}

void start()
{
	
	InitializeCriticalSection(&cs);
	vector<SmirnovSession*> sessions;
	int i = 1;

    HANDLE hStartEvent = CreateEvent(NULL, FALSE, FALSE, L"StartEvent");
    HANDLE hStopEvent = CreateEvent(NULL, FALSE, FALSE, L"StopEvent");
    HANDLE hConfirmEvent = CreateEvent(NULL, FALSE, FALSE, L"ConfirmEvent");
    HANDLE hControlEvents[2] = {hStartEvent, hStopEvent};
    while (i)
    {
        int n = WaitForMultipleObjects(2, hControlEvents, FALSE, INFINITE) - WAIT_OBJECT_0;
        switch (n)
        {
            case 0:
				sessions.push_back(new SmirnovSession(i));
                CreateThread(NULL, 0, MyThread, (LPVOID) sessions.back(), 0, NULL);
                SetEvent(hConfirmEvent);
                i++;
                break;
            case 1:
				ResetEvent(hStopEvent);

				sessions.back()->addMessage(MT_CLOSE);
				sessions.pop_back();
				i--;
                SetEvent(hConfirmEvent);
                break;
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
