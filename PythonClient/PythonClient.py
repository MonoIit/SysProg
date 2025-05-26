import threading
from dataclasses import dataclass
import socket, struct, time
from Message import *



def ProcessMessages(s):
	while True:
		m = Message.SendMessage(s, MR_BROKER, MT_GETDATA)
		if m.Header.Type == MT_DATA:
			print(m.Data)
		else:
			time.sleep(0.5)


def Client():
	HOST = 'localhost'
	PORT = 12345
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect((HOST, PORT))

	Message.SendMessage(s, MR_BROKER, MT_INIT)
	t = threading.Thread(target=ProcessMessages, args=(s,))
	t.start()
	while True:
		Message.SendMessage(s, MR_ALL, MT_DATA, input())

Client()
