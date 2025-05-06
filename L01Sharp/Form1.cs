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
     
        private static int ThreadCounter = 0;
        volatile bool _running = false;
        private Thread t;

        public Form1()
        {   
            InitializeComponent();
            textBox1.Text = "";
        }

        void ProccesClient()
        {
            while (_running)
            {
                var m = Message.send(MessageRecipients.MR_BROKER, MessageTypes.MT_GETDATA);
                switch ((MessageTypes)m.header.type)
                {
                    case MessageTypes.MT_DISCONNECT:
                        Invoke((Action)(() => {
                            textBox2.AppendText(m.data + " disconnected!" + Environment.NewLine);
                            listBox1.Items.Remove(m.data);
                        }));
                        break;
                    case MessageTypes.MT_CONNECT:
                        Invoke((Action)(() => {
                            textBox2.AppendText(m.data + " connected!" + Environment.NewLine);
                            if (!listBox1.Items.Contains(m.data))
                                listBox1.Items.Add(m.data);
                        }));
                        break;
                    case MessageTypes.MT_DATA:
                        Invoke((Action)(() => {
                            textBox2.AppendText(m.header.from + ": " + m.data + Environment.NewLine);
                        }));
                        break;
                    default:
                        Thread.Sleep(500);
                        break;
                }
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            try
            {
                if (_running) return;

                var m = Message.send(MessageRecipients.MR_BROKER, MessageTypes.MT_INIT);
                listBox1.Items.Add("Все потоки");

                var users = m.data.Split(';');
                Invoke((Action)(() => {
                    foreach (var u in users)
                        if (!listBox1.Items.Contains(u))
                            listBox1.Items.Add(u);
                }));
                

                t = new Thread(ProccesClient);
                _running = true;
                t.Start();

            }
            catch (Exception ex) { MessageBox.Show(ex.Message); }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            try
            {
                if (!_running) return;

                _running = false;
                t.Join();
                //var m = Message.send(MessageRecipients.MR_BROKER, MessageTypes.MT_EXIT);
                Native.close_socket();

                textBox1.Clear();
                textBox2.Clear();
                listBox1.Items.Clear();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Ошибка: " + ex.ToString());
            }
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            button2_Click(sender, e);
        }

        private void ChildProcess_Exited(object sender, EventArgs e)
        {
            Invoke((Action)(() =>
            {
                listBox1.Items.Clear();
                ThreadCounter = 0;
            }));
        }

        private void button3_Click(object sender, EventArgs e)
        {
            try
            {
                string dir = listBox1.SelectedItem.ToString();
                string msg = textBox1.Text;
                MessageRecipients threadId;

                if (dir == "Все потоки")
                {
                    threadId = MessageRecipients.MR_ALL;
                }
                else
                {
                    threadId = (MessageRecipients) int.Parse(dir);
                }
                Message.send(threadId, MessageTypes.MT_DATA, msg);
                //textBox2.AppendText(Message.clientID + ": " + msg + '\n');
            }
            catch (Exception ex)
            {
                MessageBox.Show("Ошибка: " + ex.ToString());
            }
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        private void textBox2_TextChanged(object sender, EventArgs e)
        {

        }
    }
}
