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
        IntPtr socket;
        private static int ThreadCounter = 0;
        
        private static ushort PORT = 12345;
        private static string IP = "127.0.0.1";

        public Form1()
        {
            socket = Native.create_socket();
            Native.connect_socket(socket, IP, PORT);

            InitializeComponent();
            textBox1.Text = "5";    
        }

        private void send(MessageTypes type, int to = -1, string data = "")
        {
            int size = Encoding.Unicode.GetByteCount(data);
            Message m = new Message(type, to, size, data);
            Native.send_header(socket, ref m.header);
            if (data.Length > 0)
            {
                IntPtr p = Marshal.StringToHGlobalUni(m.data);
                Native.send_data(socket, p, m.header.size);
                Marshal.FreeHGlobal(p);
            }
        }

        private MessageTypes receive(Message m)
        {
            Native.read_header(socket, out m.header);
            if (m.header.size > 0)
            {
                var buf = Marshal.AllocHGlobal(m.header.size);
                Native.read_data(socket, buf, m.header.size);
                m.data = Marshal.PtrToStringUni(buf, m.header.size / 2);
                Marshal.FreeHGlobal(buf);
            }
            return (MessageTypes)m.header.type;
        }

        private void redrawListbox(int threadsLength)
        {
            listBox1.Items.Clear();
            listBox1.Items.Add("Все потоки");
            listBox1.Items.Add("Главный поток");

            for (int i = 0; i < threadsLength; ++i)
            {
                listBox1.Items.Add(i + 1);
            }
            ThreadCounter = threadsLength;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            try
            {
                int N = int.Parse(textBox1.Text);
                if (ThreadCounter == 0)
                {
                    listBox1.Items.Clear();
                    listBox1.Items.Add("Все потоки");
                    listBox1.Items.Add("Главный поток");
                }


                for (int i = 0; i < N; i++)
                {
                    send(MessageTypes.MT_CREATE);
                    Message response = new Message();
                    if (receive(response) == MessageTypes.MT_CONFIRM)
                    {
                        if (response.header.to != ThreadCounter + 1) { redrawListbox(response.header.to); }
                        else { listBox1.Items.Add(++ThreadCounter); }
                    }
                }
            }
            catch (Exception ex) { MessageBox.Show(ex.Message); }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            try
            {
                    if (ThreadCounter == 0)
                    {
                        listBox1.Items.Clear();
                        Native.close_socket(socket);
                    }
                    else
                    {
                        send(MessageTypes.MT_CLOSE);
                        Message response = new Message();
                        if (receive(response) == MessageTypes.MT_CONFIRM) {
                            if (response.header.to != ThreadCounter + 1) { redrawListbox(response.header.to); }
                            else
                            {
                                listBox1.Items.RemoveAt(listBox1.Items.Count - 1);
                                ThreadCounter--;
                            }

                        }
                    }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Ошибка: " + ex.ToString());
            }
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            Native.destroy_socket(socket);
        }

        private void ChildProcess_Exited(object sender, EventArgs e)
        {
            Invoke((Action)(() =>
            {
                listBox1.Items.Clear();
                ThreadCounter = 0;
                Native.close_socket(socket);
            }));
        }

        private void button3_Click(object sender, EventArgs e)
        {
            try
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
                send(MessageTypes.MT_DATA, threadId, msg);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Ошибка: " + ex.ToString());
            }
        }
    }
}
