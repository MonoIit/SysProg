#pragma once
#include "pch.h"

enum MessageTypes
{
	MT_INIT,
	MT_EXIT,
	MT_CREATE,
	MT_CLOSE,
	MT_DATA,
	MT_CONFIRM,
};

struct Header {
	int type;
	int to;
	int size;
};

class Message {
public:
	Header header;
	std::wstring data;
	Message() = default;

	Message(int type, int to = -1, const wstring& data = L"")
	{
		this->data = data;
		header = { type, to, int(data.length() * sizeof(wchar_t)) };
	}
};

