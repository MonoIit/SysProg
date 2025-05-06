using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace L01Sharp
{
    static class Native
    {
        [DllImport("MMF.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern IntPtr connect_socket(string host, ushort port);

        [DllImport("MMF.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool send_header(IntPtr sock, ref Header h);

        [DllImport("MMF.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool send_data(IntPtr sock, IntPtr buf, int bytes);

        [DllImport("MMF.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool read_header(IntPtr sock, out Header h);

        [DllImport("MMF.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool read_data(IntPtr sock, IntPtr buf, int bytes);

        [DllImport("MMF.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void close_socket();

        [DllImport("MMF.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void destroy_socket(IntPtr sock);
    }
}
