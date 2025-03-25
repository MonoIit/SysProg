using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;

namespace L01Sharp
{
    public partial class Form1 : Form
    {
        [DllImport("MMF.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool InitSharedMemory();

        [DllImport("MMF.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
        public static extern bool WriteData(int threadID, string data);

        [DllImport("MMF.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Cleanup();


        Process childProcess = null;
        System.Threading.EventWaitHandle stopEvent = new EventWaitHandle(false, EventResetMode.ManualReset, "StopEvent");
        System.Threading.EventWaitHandle startEvent = new EventWaitHandle(false, EventResetMode.AutoReset, "StartEvent");
        System.Threading.EventWaitHandle sendEvent = new EventWaitHandle(false, EventResetMode.ManualReset, "SendEvent");
        System.Threading.EventWaitHandle confirmEvent = new EventWaitHandle(false, EventResetMode.AutoReset, "ConfirmEvent");
        int ThreadCounter = 0;

        public Form1()
        {
            InitSharedMemory();
            InitializeComponent();
            textBox1.Text = "5";
        }

        private void button1_Click(object sender, EventArgs e)
        {
            try
            {
                int N = int.Parse(textBox1.Text);
                if (childProcess == null || childProcess.HasExited)
                {
                    listBox1.Items.Clear();
                    ThreadCounter = 0;
                    childProcess = new Process();
                    childProcess.StartInfo.FileName = "L01.exe";
                    childProcess.EnableRaisingEvents = true;
                    childProcess.Exited += ChildProcess_Exited;

                    childProcess.Start();

                    listBox1.Items.Add("Все потоки");
                    listBox1.Items.Add("Главный поток");
                }


                for (int i = 0; i < N; i++)
                {
                    startEvent.Set();
                    if (confirmEvent.WaitOne())
                    {
                        listBox1.Items.Add(++ThreadCounter);
                    }
                }
            }
            catch (Exception ex) { MessageBox.Show(ex.Message); }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            try
            {
                if (!(childProcess == null || childProcess.HasExited))
                {
                    if (ThreadCounter == 0)
                    {
                        listBox1.Items.Clear();
                        childProcess.Kill();
                    }
                    else
                    {
                        stopEvent.Set();
                        if (confirmEvent.WaitOne())
                        {
                            listBox1.Items.RemoveAt(listBox1.Items.Count - 1);
                            ThreadCounter--;
                        }
                    }
                }
                else
                {
                    listBox1.Items.Clear();
                    ThreadCounter = 0;
                }
            }
            catch (Exception ex) { MessageBox.Show(ex.ToString()); }
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            while (!(childProcess == null || childProcess.HasExited))
            {
                button2_Click(sender, e);
            }
            Cleanup();
        }

        private void ChildProcess_Exited(object sender, EventArgs e)
        {
            Invoke((Action)(() =>
            {
                listBox1.Items.Clear();
                ThreadCounter = 0;
                childProcess = null;
            }));
        }

        private void button3_Click(object sender, EventArgs e)
        {
            if (!(childProcess == null || childProcess.HasExited))
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
                    WriteData(threadId, msg);
                    sendEvent.Set();
                    if (confirmEvent.WaitOne())
                    {

                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.ToString());
                }
            }

        }
    }
}
