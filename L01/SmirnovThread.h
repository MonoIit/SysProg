#pragma once

#include "SmirnovSession.h"
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

    void addMessage(MessageTypes messageType, const string& data = "") {
        session->addMessage(messageType, data);
    }

    void run() {
        cout << "Session " << session->sessionID << " created" << endl;
        while (isRunning) {
            Message m;
            if (session->getMessage(m)) {
                switch (m.header.messageType) {
                case MT_CLOSE:
                    SafeWrite("session", session->sessionID, "закрыта");
                    isRunning = false;
                    break;
                case MT_DATA:
                    SafeWrite("session", session->sessionID, "data", m.data);
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
