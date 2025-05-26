from dataclasses import dataclass
import socket, struct, time, sys

MT_INIT		  = 0
MT_EXIT		  = 1
MT_CREATE     = 2
MT_CLOSE      = 3
MT_DATA		  = 4
MT_CONFIRM	  = 5
MT_GETDATA	  = 6
MT_NODATA	  = 7
MT_CONNECT    = 8
MT_DISCONNECT = 9


MR_BROKER	= 10
MR_ALL		= 50
MR_USER		= 100


@dataclass
class MsgHeader:
	Type: int = 0
	To: int = 0
	From: int = 0
	Size: int = 0

	def Send(self, s):
		s.send(struct.pack(f'iiii', self.Type, self.To, self.From, self.Size))

	def Receive(self, s):
		try:
			(self.Type, self.To, self.From, self.Size) = struct.unpack('iiii', s.recv(16))
		except:
			self.Size = 0
			self.Type = MT_NODATA

class Message:
	ClientID = 0

	def __init__(self, To, From, Type = MT_DATA, Data=""):
		self.Header = MsgHeader(Type, To, From, len(Data) * 2)
		self.Data = Data

	def Send(self, s):
		self.Header.Send(s)
		if self.Header.Size != 0:
			s.send(struct.pack(f'{self.Header.Size}s', self.Data.encode('utf-16-le')))

	def Receive(self, s):
		self.Header.Receive(s)
		if self.Header.Size != 0:
			self.Data = struct.unpack(f'{self.Header.Size}s', s.recv(self.Header.Size))[0].decode('utf-16-le')
		return self.Header.Type

	@staticmethod
	def SendMessage(socket, To, Type = MT_DATA, Data=""):
		m = Message(To, Message.ClientID, Type, Data)
		m.Send(socket)
		if m.Receive(socket) == MT_INIT:
			Message.ClientID = m.Header.To
		return m

