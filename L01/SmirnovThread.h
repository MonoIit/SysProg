#pragma once

#include "pch.h"
#include "SmirnovSession.h"
#include "SysProg.h"


class SmirnovThread {
private:
    static atomic<int> threadCounter;
    SmirnovSession* session;
    thread worker;
    bool isRunning = true;
    bool haveFile = false;
    int id;

public:
    explicit SmirnovThread(int id)
        : session(new SmirnovSession(id)), worker(&SmirnovThread::run, this), id(id) {
    }

    int getThreadId() {
        return id;
    }

    SmirnovSession* getSession() {
        return session;
    }

    void addMessage(Message& m) {
        session->addMessage(m);
    }

    void run() {
        SafeWrite(L"Session", session->sessionID, "created");

        while (isRunning) {
            /*Message m;
            if (session->getMessage(m)) {
                switch (m.header.type) {

                case MT_CLOSE:
                    SafeWrite("session", session->sessionID, "closed");
                    isRunning = false;
                    break;
                case MT_DATA:
                    SafeWrite("session", session->sessionID, "data", m.data);
                    
                    wofstream outFile(to_wstring(session->sessionID) + L".txt", ios::binary | ios::app);
                    
                    if (outFile.is_open()) {
                        outFile.imbue(locale(outFile.getloc(),
                            new codecvt_utf16<wchar_t, 0x10ffff, little_endian>));

                        if (!haveFile) {
                            wchar_t bom = 0xFEFF;
                            outFile.write(&bom, 1);
                            haveFile = true;
                        }

                        outFile << m.data << L"\n";
                        outFile.close();
                    }

                    this_thread::sleep_for(chrono::milliseconds(500 * session->sessionID));
                    break;
                }
            }*/
        }
    }

    void stop() {
        isRunning = false;
        if (worker.joinable()) {
            worker.join();
        }
        delete session;
    }

    ~SmirnovThread() {
        stop();
    }
};
