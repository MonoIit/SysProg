#pragma once
#pragma comment(lib, "MMF.lib")

#include "pch.h"
#include "mmf.h"

using boost::asio::ip::tcp;


enum MessageTypes
{
	MT_INIT,
	MT_EXIT,
	MT_CREATE,
	MT_CLOSE,
	MT_DATA,
	MT_CONFIRM,
	MT_GETDATA,
	MT_NODATA,
	MT_CONNECT,
	MT_DISCONNECT,
};

enum MessageRecipients
{
	MR_BROKER = 10,
	MR_ALL = 50,
	MR_USER = 100
};



class Message {
public:
	Header header = { 0 };
	std::wstring data;
	static int clientID;

	Message() {};

	Message(int type, int to = -1, int from = -1, const wstring& data = L"")
	{
		this->data = data;
		header = { type, to, from, (int) data.length() * 2 };
	}

	int receive(tcp::socket& s) {
		read_header(&s, &header);
		if (header.size) {
			data.resize(header.size / sizeof(wchar_t));
			read_data(&s, const_cast<wchar_t*>(data.data()), header.size);
		}
		return header.type;
	}

	void send(tcp::socket& s) {
		send_header(&s, &header);
		if (header.size)
		{
			send_data(&s, data.c_str(), header.size);
		}
	}

	static void send(tcp::socket& s, int to, int from, int type = MT_DATA, const wstring& data = L"");
	static Message send(int to, int type = MT_DATA, const wstring& data = L"");
};

