using System;
using System.Collections.Generic;
using System.Linq;
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
    };


    [StructLayout(LayoutKind.Sequential)]
    public struct Header
    {
        public int type;
        public int to;
        public int size;
    }




    public class Message
    {
        public Header header;
        public string data;

        public Message() { }

        public Message(MessageTypes type, int to = 1, int size = 0, string data = "")
        {
            header = new Header { type = (int)type, to = to, size = size };
            this.data = data;
        }
    }
}
