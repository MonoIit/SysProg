#include "pch.h"
#include "mmf.h"
#include <boost/system/system_error.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>

boost::asio::io_context global_io;
tcp::socket global_socket(global_io);

tcp::socket* connect_socket(const char* host, unsigned short port) {
    try {
        if (!global_socket.is_open()) {
            tcp::resolver resolver(global_io);
            auto endpoints = resolver.resolve(host, std::to_string(port));
            boost::asio::connect(global_socket, endpoints);
        }
        return &global_socket;
    }
    catch (const std::exception& e) {
        std::cerr << "Connection failed: " << e.what() << std::endl;
        return nullptr;
    }
}

bool send_header(tcp::socket* s, const Header* h) {
    boost::system::error_code ec;
    boost::asio::write(*s, boost::asio::buffer(h, sizeof(Header)), ec);
    if (ec) throw boost::system::system_error(ec);
    return true;
}

bool send_data(tcp::socket* s, const wchar_t* buf, int bytes) {
    boost::system::error_code ec;
    boost::asio::write(*s, boost::asio::buffer(buf, bytes), ec);
    if (ec) throw boost::system::system_error(ec);
    return true;
}

bool read_header(tcp::socket* s, Header* h) {
    boost::system::error_code ec;
    boost::asio::read(*s, boost::asio::buffer(h, sizeof(Header)), ec);
    if (ec) throw boost::system::system_error(ec);
    return true;
}

bool read_data(tcp::socket* s, wchar_t* buf, int bytes) {
    boost::system::error_code ec;
    boost::asio::read(*s, boost::asio::buffer(buf, bytes), ec);
    if (ec) throw boost::system::system_error(ec);
    return true;
}

void close_socket() {
    boost::system::error_code ec;
    global_socket.close(ec);
}

void destroy_socket(tcp::socket* s) {
    delete s;
}