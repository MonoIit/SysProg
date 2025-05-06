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

        void send(IntPtr socket)
        {
            Native.send_header(socket, ref header);
            if (data.Length > 0)
            {
                IntPtr p = Marshal.StringToHGlobalUni(data);
                Native.send_data(socket, p, header.size);
                Marshal.FreeHGlobal(p);
            }
        }

        MessageTypes receive(IntPtr socket)
        {
            Native.read_header(socket, out header);
            if (header.size > 0)
            {
                var buf = Marshal.AllocHGlobal(header.size);
                Native.read_data(socket, buf, header.size);
                data = Marshal.PtrToStringUni(buf, header.size / 2);
                Marshal.FreeHGlobal(buf);
            }
            return (MessageTypes)header.type;
        }

        static public void send(IntPtr socket, MessageRecipients to, MessageRecipients from, MessageTypes type = MessageTypes.MT_DATA, string data = "")
        {
            new Message(to, from, type, data).send(socket);
        }

        static public Message send(MessageRecipients to, MessageTypes type = MessageTypes.MT_DATA, string data = "")
        {
            IntPtr s = Native.connect_socket(IP, PORT);
            
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
