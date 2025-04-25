#pragma once
#include "pch.h"
#include <boost/asio.hpp>
using boost::asio::ip::tcp;


struct Header {
    int type;
    int to;
    int size;
};


extern "C" {
    __declspec(dllexport) tcp::socket* create_socket();
    __declspec(dllexport) bool          connect_socket(tcp::socket* s, const char* host, unsigned short port);
    __declspec(dllexport) bool          send_header(tcp::socket* s, const Header* h);
    __declspec(dllexport) bool          send_data(tcp::socket* s, const wchar_t* buf, int bytes);
    __declspec(dllexport) bool          read_header(tcp::socket* s, Header* h);
    __declspec(dllexport) bool          read_data(tcp::socket* s, wchar_t* buf, int bytes);
    __declspec(dllexport) void          close_socket(tcp::socket* s);
    __declspec(dllexport) void          destroy_socket(tcp::socket* s);
}