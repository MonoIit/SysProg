#include "pch.h"
#include "mmf.h"
#include <boost/system/system_error.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>

tcp::socket* create_socket() {
    static boost::asio::io_context ios;
    return new tcp::socket(ios);
}

bool connect_socket(tcp::socket* s, const char* host, unsigned short port) {
    boost::asio::ip::tcp::resolver resolver{s->get_executor()};
    auto endpoints = resolver.resolve(host, std::to_string(port));
    boost::asio::connect(*s, endpoints);
    return true;
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

void close_socket(tcp::socket* s) {
    boost::system::error_code ec;
    s->close(ec);
}

void destroy_socket(tcp::socket* s) {
    delete s;
}