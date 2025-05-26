using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace L01Sharp
{
    public enum MessageTypes : int
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
        MT_DISCONNECT
    };


    public enum MessageRecipients : int
    {
        MR_BROKER = 10,
        MR_ALL = 50,
        MR_USER = 100
    };


    [StructLayout(LayoutKind.Sequential)]
    public struct Header
    {
        public int type;
        public int to;
        public int from;
        public int size;
    }




    public class Message
    {
        private static ushort PORT = 12345;
        private static string IP = "127.0.0.1";

        public Header header;
        public string data;
        public static MessageRecipients clientID;

        public Message(MessageRecipients to, MessageRecipients from, MessageTypes type = MessageTypes.MT_DATA, string data = "")
        {
            header = new Header { type = (int)type, to = (int)to, from = (int)from, size = data.Length * 2 };
            this.data = data;
        }

        static byte[] toBytes(object obj)
        {
            int size = Marshal.SizeOf(obj);
            byte[] buff = new byte[size];
            IntPtr ptr = Marshal.AllocHGlobal(size);
            Marshal.StructureToPtr(obj, ptr, true);
            Marshal.Copy(ptr, buff, 0, size);
            Marshal.FreeHGlobal(ptr);
            return buff;
        }

        static T fromBytes<T>(byte[] buff) where T : struct
        {
            T data = default(T);
            int size = Marshal.SizeOf(data);
            IntPtr i = Marshal.AllocHGlobal(size);
            Marshal.Copy(buff, 0, i, size);
            var d = Marshal.PtrToStructure(i, data.GetType());
            if (d is not null)
            {
                data = (T)d;
            }
            Marshal.FreeHGlobal(i);
            return data;
        }

        void send(Socket s)
        {
            s.Send(toBytes(header), Marshal.SizeOf(header), SocketFlags.None);
            if (header.size != 0)
            {
                s.Send(Encoding.Unicode.GetBytes(data), header.size, SocketFlags.None);
            }
        }

        MessageTypes receive(Socket socket)
        {
            byte[] buff = new byte[Marshal.SizeOf(header)];
            if (socket.Receive(buff, Marshal.SizeOf(header), SocketFlags.None) == 0)
            {
                return MessageTypes.MT_NODATA;
            }
            header = fromBytes<Header>(buff);
            if (header.size > 0)
            {
                byte[] b = new byte[header.size];
                socket.Receive(b, header.size, SocketFlags.None);
                data = Encoding.Unicode.GetString(b, 0, header.size);

            }
            return (MessageTypes) header.type;
        }

        static public void send(Socket socket, MessageRecipients to, MessageRecipients from, MessageTypes type = MessageTypes.MT_DATA, string data = "")
        {
            new Message(to, from, type, data).send(socket);
        }

        static public Message send(MessageRecipients to, MessageTypes type = MessageTypes.MT_DATA, string data = "")
        {
            int nPort = 12345;
            IPEndPoint endPoint = new IPEndPoint(IPAddress.Parse("127.0.0.1"), nPort);
            Socket s = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            s.Connect(endPoint);
            if (!s.Connected)
            {
                throw new Exception("Connection error");
            }
            var m = new Message(to, clientID, type, data);
            m.send(s);
            if (m.receive(s) == MessageTypes.MT_INIT)
            {
                clientID = (MessageRecipients) m.header.to;
            }
            return m;
        }

    }
}
