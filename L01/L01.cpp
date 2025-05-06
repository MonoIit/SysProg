#pragma once
#pragma comment(lib, "MMF.lib")

#include "pch.h"
#include "SmirnovSession.h"
#include "SmirnovThread.h"
#include "Message.h"

using boost::asio::ip::tcp;


int maxID = MR_USER;
map<int, shared_ptr<SmirnovThread>> threads;
unordered_map<int, chrono::steady_clock::time_point> lastSeen;
mutex timeMutex;

void processClient(tcp::socket s) {
    try {
        while (true) {
            Message m;
            int code = m.receive(s);
            
            if (m.header.from != 0) {
                lock_guard<std::mutex> lock(timeMutex);
                lastSeen[m.header.from] = chrono::steady_clock::now();
                //cout << m.header.to << ": " << m.header.from << ": " << m.header.type << ": " << code << endl;
            }

            switch (code) {
            case (MT_INIT): {
                auto thread = make_shared<SmirnovThread>(++maxID);
                int newId = thread->getThreadId();
                threads[newId] = thread;

                wstring allUsers = L"";
                for (auto& pair : threads) {
                    allUsers.append(to_wstring(pair.first) + L";");
                }

                if (!allUsers.empty())
                    allUsers.pop_back();

                Message::send(s, newId, MR_BROKER, MT_INIT, allUsers);

                for (auto& pair : threads) {
                    Message mes(MT_CONNECT, pair.first, MR_BROKER, to_wstring(newId));
                    pair.second->addMessage(mes);
                }

                break;
            }
            //case (MT_EXIT): {
            //    int exitedId = m.header.from;
            //    auto it = threads.find(exitedId);
            //    if (it != threads.end()) {
            //        threads.erase(it);
            //    }
            //    Message::send(s, exitedId, MR_BROKER, MT_EXIT);

            //    for (auto& pair : threads) {
            //        Message mes(MT_DISCONNECT, pair.first, MR_BROKER, to_wstring(exitedId) + L" disconnected(");
            //        pair.second->addMessage(mes);
            //    }

            //    break;
            //}
            case (MT_GETDATA): {
                auto iSession = threads.find(m.header.from);
                if (iSession != threads.end())
                {
                    iSession->second->getSession()->send(s);
                }
                break;
            }
            case (MT_DATA): {
                wstring msg = m.data;
                if (!msg.empty() && msg[0] == L' ') msg.erase(0, 1);


                int idFrom = m.header.from;
                int idTo = m.header.to;
                auto iThreadFrom = threads.find(idFrom);
                if (iThreadFrom != threads.end()) {
                    auto iThreadTo = threads.find(idTo);
                    if (iThreadTo != threads.end())
                    {
                        iThreadTo->second->addMessage(m);
                    }
                    else if (idTo == MR_ALL)
                    {
                        for (auto& pair : threads)
                        {
                            int id = pair.first;
                            auto& thread = pair.second;
                          
                             thread->addMessage(m);
                        }
                    }
                    Message::send(s, m.header.from, MR_BROKER, MT_CONFIRM);
                }
                break;
            }
            default:
                break;
            }



        }
    }
    catch (std::exception& e) {
        std::wcerr << L"Exception in client handler: " << e.what() << std::endl;
    }
}

void deleteSleepingUsers() {
    while (true) {
        this_thread::sleep_for(chrono::seconds(5));

        auto now = chrono::steady_clock::now();
        vector<int> toRemove;

        {
            lock_guard<std::mutex> lock(timeMutex);
            for (auto& pair : lastSeen) {
                auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - pair.second);
                if (diff.count() > 10) {
                    toRemove.push_back(pair.first);
                }
            }
        }

        for (int id : toRemove) {
            {
                std::lock_guard<std::mutex> lock(timeMutex);
                lastSeen.erase(id);
            }

            auto it = threads.find(id);
            if (it != threads.end()) {
                threads.erase(it);
                SafeWrite(to_wstring(id) + L" delete");
            }

            for (auto& pair : threads) {
                Message mes(MT_DISCONNECT, pair.first, MR_BROKER, to_wstring(id));
                pair.second->addMessage(mes);
            }
        }
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

    
    thread(deleteSleepingUsers).detach();
    startServer();
	return 0;
}
