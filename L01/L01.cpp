#pragma once

#include "pch.h"
#include "SmirnovSession.h"
#include "SmirnovThread.h"
#include "Message.h"

using boost::asio::ip::tcp;

typedef tcp::socket* (*create_socket_func)();
typedef bool (*connect_socket_func)(tcp::socket*, const char*, unsigned short);
typedef bool (*send_header_func)(tcp::socket*, const Header*);
typedef bool (*send_data_func)(tcp::socket*, const wchar_t*, int);
typedef bool (*read_header_func)(tcp::socket*, Header*);
typedef bool (*read_data_func)(tcp::socket*, wchar_t*, int);
typedef void (*close_socket_func)(tcp::socket*);
typedef void (*destroy_socket_func)(tcp::socket*);


HMODULE dll = LoadLibraryA("MMF.dll");

auto create_socket = (create_socket_func)GetProcAddress(dll, "create_socket");
auto connect_socket = (connect_socket_func)GetProcAddress(dll, "connect_socket");
auto send_header = (send_header_func)GetProcAddress(dll, "send_header");
auto send_data = (send_data_func)GetProcAddress(dll, "send_data");
auto read_header = (read_header_func)GetProcAddress(dll, "read_header");
auto read_data = (read_data_func)GetProcAddress(dll, "read_data");
auto close_socket = (close_socket_func)GetProcAddress(dll, "close_socket");
auto destroy_socket = (destroy_socket_func)GetProcAddress(dll, "destroy_socket");



int receive(tcp::socket& s, Message& m) {
    read_header(&s, &m.header);
    if (m.header.size) {
        m.data.resize(m.header.size / sizeof(wchar_t));
        read_data(&s, const_cast<wchar_t*>(m.data.data()), m.header.size);
    }
    return m.header.type;
}

void send(tcp::socket& s, Message& m) {
    send_header(&s, &m.header);
    if (m.header.size)
    {
        send_data(&s, m.data.c_str(), m.header.size);
    }
}


vector<SmirnovThread*> threads;

void processClient(tcp::socket s) {
    try {
        while (true) {
            Message m(MT_DATA);
            int code = receive(s, m);
            switch (code) {
            case (MT_INIT): {

                break;
            }
            case (MT_EXIT): {

                break;
            }
            case (MT_CREATE): {
                auto thread = new SmirnovThread();
                threads.push_back(thread);

                int count = SmirnovThread::getThreadCounter();
                Message response(MT_CONFIRM, threads.size());
                send(s, response);
                break;
            }
            case (MT_CLOSE): {
                if (!threads.empty()) {
                    threads.back()->addMessage(MT_CLOSE);
                    delete threads.back();
                    threads.pop_back();

                    int count = SmirnovThread::getThreadCounter();
                    Message response(MT_CONFIRM, threads.size());
                    send(s, response);
                }
                else {
                    Message response(MT_CONFIRM, threads.size());
                    send(s, response);
                }
                break;
            }
            case (MT_DATA): {
                wstring msg = m.data;
                if (!msg.empty() && msg[0] == L' ') msg.erase(0, 1);

                
                int id = m.header.to;
                if (id >= 0 && id < threads.size()) {
                    threads[id]->addMessage(MT_DATA, id, msg);
                }
                else if (id == -1) {
                    SafeWrite(L"Главный поток:", msg);
                }
                else if (id == -2) {
                    for (auto thread : threads) {
                        thread->addMessage(MT_DATA, id, msg);
                    }
                }
                Message response(MT_CONFIRM, threads.size());
                send(s, response);
                break;
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

    

    startServer();
	return 0;
}
