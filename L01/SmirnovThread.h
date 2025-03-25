#pragma once

#include "SmirnovSession.h"
#include <fstream>
#include <thread>
#include <mutex>
#include <atomic>

class SmirnovThread {
private:
    static atomic<int> threadCounter;
    SmirnovSession* session;
    thread worker;
    bool isRunning = true;

public:
    explicit SmirnovThread()
        : session(new SmirnovSession(++threadCounter)), worker(&SmirnovThread::run, this) {
    }

    void addMessage(MessageTypes messageType, const wstring data = L" ") {
        session->addMessage(messageType, data);
    }

    void run() {
        SafeWrite(L"Session", session->sessionID, "created");
        while (isRunning) {
            Message m;
            if (session->getMessage(m)) {
                switch (m.header.messageType) {
                case MT_CLOSE:
                    SafeWrite("session", session->sessionID, "closed");
                    isRunning = false;
                    break;
                case MT_DATA:
                    // SafeWrite("session", session->sessionID, "data", m.data);

                    wstring filename = to_wstring(session->sessionID) + L".txt";
                    wofstream outFile(filename, ios::binary);

                    if (outFile.is_open()) {
                        outFile << m.data << " ";
                        outFile.close();
                    }

                    this_thread::sleep_for(chrono::milliseconds(500 * session->sessionID));
                    break;
                }
            }
        }
    }

    void stop() {
        isRunning = false;
        if (worker.joinable()) {
            worker.join();
            threadCounter--;
        }
        delete session;
    }

    ~SmirnovThread() {
        stop();
    }
};
