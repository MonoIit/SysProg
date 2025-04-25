#pragma once

#ifndef _WIN32_WINNT
#define	_WIN32_WINNT	0x0A00
#endif						

#include "pch.h"



inline wstring receiveString(tcp::socket& s)
{
	int n = receiveInt(s);
	wstring str;
	str.resize(n / sizeof(wchar_t));

	boost::system::error_code error;
	boost::asio::read(s, boost::asio::buffer(&str[0], n), error);
	if (error)
		throw boost::system::system_error(error);

	return str;
}

inline void sendString(tcp::socket& s, const wstring& str)
{
	int n = int(str.length() * sizeof(wchar_t));
	sendInt(s, n);

	boost::system::error_code error;
	boost::asio::write(s, boost::asio::buffer(str.data(), n), error);
	if (error)
		throw boost::system::system_error(error);
}

