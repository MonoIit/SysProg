#include "pch.h"
#include "SmirnovSession.h"
#include "SmirnovThread.h"
#include "asio.h"

using namespace std;

//struct header {
//    int addr;
//    int size;
//};
//
//typedef wstring (*ReadDataFunc)(header& h);

//void start(HMODULE hDLL, ReadDataFunc ReadData)
//{
//	InitializeCriticalSection(&cs);
//
//	vector<SmirnovThread*> threads;
//    boolean flag = true;
//
//    HANDLE hStartEvent = CreateEvent(NULL, FALSE, FALSE, L"StartEvent");
//    HANDLE hStopEvent = CreateEvent(NULL, FALSE, FALSE, L"StopEvent");
//    HANDLE hSendEvent = CreateEvent(NULL, FALSE, FALSE, L"SendEvent");
//    HANDLE hConfirmEvent = CreateEvent(NULL, FALSE, FALSE, L"ConfirmEvent");
//    HANDLE hControlEvents[3] = {hStartEvent, hStopEvent, hSendEvent};
//    while (flag)
//    {
//        int n = WaitForMultipleObjects(3, hControlEvents, FALSE, INFINITE) - WAIT_OBJECT_0;
//        
//        switch (n)
//        {
//        case 0: {
//            
            /*auto thread = new SmirnovThread();
            threads.push_back(thread);*/
//            SetEvent(hConfirmEvent);
//            break;
//        }
//        case 1: {
//            ResetEvent(hStopEvent);
//            if (!threads.empty()) {
//                threads.back()->addMessage(MT_CLOSE);
//                delete threads.back();
//                threads.pop_back();
//            }
//            else {
//                flag = false;
//            }
//            SetEvent(hConfirmEvent);
//            break;
//        }
//        case 2: {
//            ResetEvent(hSendEvent);
//
//            header h;
//            wstring msg = ReadData(h);
//
//
//            if (h.addr == -1) {
//                SafeWrite(L"Главный поток:", msg);
//            }
//            else if (h.addr == -2) {
//                for (auto thread : threads) {
//                    thread->addMessage(MT_DATA, msg);
//                }
//            }
//            else if (!threads.empty()) {
//                threads[h.addr]->addMessage(MT_DATA, msg);
//            }
//
//            SetEvent(hConfirmEvent);
//            break;
//        }
//        }
//    }
//    SetEvent(hConfirmEvent);
//	DeleteCriticalSection(&cs);
//}

vector<SmirnovThread*> threads;

void processClient(tcp::socket s) {
    try {
        while (true) {
            wstring cmd = receiveString(s);
            wistringstream wiss(cmd);
            wstring action;
            wiss >> action;

            if (action == L"CREATE") {
                auto thread = new SmirnovThread();
                threads.push_back(thread);

                int count = SmirnovThread::getThreadCounter();
                sendString(s, L"ok");
            }
            else if (action == L"CLOSE") {
                if (!threads.empty()) {
                    threads.back()->addMessage(MT_CLOSE);
                    delete threads.back();
                    threads.pop_back();

                    int count = SmirnovThread::getThreadCounter();
                    sendString(s, L"ok");
                }
                else {
                    sendString(s, L"error");
                }
            }
            else if (action == L"SEND") {
                int id;
                wiss >> id;
                wstring msg;
                getline(wiss, msg);
                if (!msg.empty() && msg[0] == L' ') msg.erase(0, 1);

                if (id >= 0 && id < threads.size()) {
                    threads[id]->addMessage(MT_DATA, msg);
                    sendString(s, L"ok");
                }
                else if (id == -1) {
                    SafeWrite(L"Главный поток:", msg);
                    sendString(s, L"ok");
                }
                else if (id == -2) {
                    for (auto thread : threads) {
                        thread->addMessage(MT_DATA, msg);
                    }
                    sendString(s, L"ok");
                }
                else {
                    sendString(s, L"error");
                }
            }
        }
    }
    catch (std::exception& e) {
        std::wcerr << L"Exception in client handler: " << e.what() << std::endl;
    }
}


void startServer() {
    try {
        int port = 12345;
        boost::asio::io_context io;
        tcp::acceptor a(io, tcp::endpoint(tcp::v4(), port));

        while (true) {
            std::thread(processClient, a.accept()).detach();
        }
    }
    catch (std::exception& e) {
        std::wcerr << L"Exception in server: " << e.what() << std::endl;
    }
}

int main()
{
    std::locale::global(std::locale("rus_rus.866"));
    wcin.imbue(std::locale());
    wcout.imbue(std::locale());

    //HMODULE hDLL = LoadLibraryA("MMF.dll");

    //ReadDataFunc ReadData = (ReadDataFunc)GetProcAddress(hDLL, "ReadData");

    startServer();
	return 0;
}
