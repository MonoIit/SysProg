using System.Diagnostics;
using System.Net;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;


namespace L01Sharp
{
    public partial class Form1 : Form
    {
        //[DllImport("MMF.dll", CallingConvention = CallingConvention.Cdecl)]
        //public static extern bool InitSharedMemory();

        //[DllImport("MMF.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        //public static extern bool WriteData(int threadID, string data);

        //[DllImport("MMF.dll", CallingConvention = CallingConvention.Cdecl)]
        //public static extern void Cleanup();


        //Process childProcess = null;
        //System.Threading.EventWaitHandle stopEvent = new EventWaitHandle(false, EventResetMode.ManualReset, "StopEvent");
        //System.Threading.EventWaitHandle startEvent = new EventWaitHandle(false, EventResetMode.AutoReset, "StartEvent");
        //System.Threading.EventWaitHandle sendEvent = new EventWaitHandle(false, EventResetMode.ManualReset, "SendEvent");
        //System.Threading.EventWaitHandle confirmEvent = new EventWaitHandle(false, EventResetMode.AutoReset, "ConfirmEvent");
        private IPEndPoint endPoint;
        private Socket s;
        private static int ThreadCounter = 0;
        
        private static int PORT = 12345;
        private static string IP = "127.0.0.1";

        public Form1()
        {
            //InitSharedMemory();

            InitializeComponent();
            textBox1.Text = "5";
        }

        private void button1_Click(object sender, EventArgs e)
        {
            try
            {
                int N = int.Parse(textBox1.Text);
                if (s == null || !s.Connected)
                {
                    endPoint = new IPEndPoint(IPAddress.Parse(IP), PORT);
                    s = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                    s.Connect(endPoint);

                    listBox1.Items.Clear();
                    ThreadCounter = 0;
                    listBox1.Items.Add("Все потоки");
                    listBox1.Items.Add("Главный поток");
                }


                for (int i = 0; i < N; i++)
                {
                    SendString(s, "CREATE");
                    listBox1.Items.Add(++ThreadCounter);
                }
            }
            catch (Exception ex) { MessageBox.Show(ex.Message); }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            try
            {
                if (s.Connected)
                {
                    if (ThreadCounter == 0)
                    {
                        listBox1.Items.Clear();
                        s.Close();
                    }
                    else
                    {
                        //string dir = listBox1.SelectedItem.ToString();

                        SendString(s, "CLOSE");

                        listBox1.Items.RemoveAt(listBox1.Items.Count - 1);
                        ThreadCounter--;
                    }
                }
                else
                {
                    listBox1.Items.Clear();
                    ThreadCounter = 0;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Ошибка: " + ex.ToString());
            }
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            while (s.Connected)
            {
                button2_Click(sender, e);
            }
            //Cleanup();
        }

        private void ChildProcess_Exited(object sender, EventArgs e)
        {
            Invoke((Action)(() =>
            {
                listBox1.Items.Clear();
                ThreadCounter = 0;
                s.Close();
            }));
        }

        private void button3_Click(object sender, EventArgs e)
        {
            try
            {
                if (s.Connected)
                {
                    string dir = listBox1.SelectedItem.ToString();
                    string msg = textBox1.Text;
                    int threadId;

                    if (dir == "Все потоки")
                    {
                        threadId = -2;
                    }
                    else if (dir == "Главный поток")
                    {
                        threadId = -1;
                    }
                    else
                    {
                        threadId = int.Parse(dir) - 1;
                    }

                    SendString(s, $"SEND {threadId} {msg}");
                }
                else
                {
                    MessageBox.Show("Нет соединения с сервером.");
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Ошибка: " + ex.ToString());
            }
        }

        static void SendString(Socket s, string str)
        {
            int n = str.Length * 2;
            s.Send(BitConverter.GetBytes(n), sizeof(int), SocketFlags.None);
            s.Send(Encoding.Unicode.GetBytes(str), n, SocketFlags.None);
        }

        static string ReceiveString(Socket s)
        {
            byte[] b = new byte[sizeof(int)];

            s.Receive(b, sizeof(int), SocketFlags.None);
            int n = BitConverter.ToInt32(b, 0);
            b = new byte[n];
            s.Receive(b, n, SocketFlags.None);

            return Encoding.Unicode.GetString(b, 0, n);
        }
    }
}
