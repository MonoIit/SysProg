#pragma once
#include "pch.h"
#include "SysProg.h"
#include "Message.h"

class SmirnovSession
{
	queue<Message> messages;
	mutex mx;
	HANDLE hEvent;
public:
	int sessionID;

	SmirnovSession(int sessionID)
		:sessionID(sessionID)
	{}

	void addMessage(Message& m)
	{
		lock_guard<mutex> lg(mx);
		messages.push(m);
	}

	bool getMessage(Message& m)
	{
		lock_guard<mutex> lg(mx);
		bool res = false;
		if (!messages.empty())
		{
			res = true;
			m = messages.front();
			messages.pop();
		}
		return res;
	}

	void send(tcp::socket& s)
	{
		lock_guard<mutex> lg(mx);
		
		if (messages.empty())
		{
			Message::send(s, sessionID, MR_BROKER, MT_NODATA);
		}
		else
		{
			messages.front().send(s);
			messages.pop();
		}
	}
};

