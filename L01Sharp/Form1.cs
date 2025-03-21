using System.Diagnostics;

namespace L01Sharp
{
    public partial class Form1 : Form
    {
        Process childProcess = null;
        System.Threading.EventWaitHandle stopEvent = new EventWaitHandle(false, EventResetMode.ManualReset, "StopEvent");
        System.Threading.EventWaitHandle startEvent = new EventWaitHandle(false, EventResetMode.AutoReset, "StartEvent");
        System.Threading.EventWaitHandle confirmEvent = new EventWaitHandle(false, EventResetMode.AutoReset, "ConfirmEvent");
        int ThreadCounter = 0;

        public Form1()
        {
            InitializeComponent();
            textBox1.Text = "1";
        }

        private void button1_Click(object sender, EventArgs e)
        {
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

            int N = int.Parse(textBox1.Text);
            for (int i = 0; i < N; i++)
            {
                startEvent.Set();
                if (confirmEvent.WaitOne())
                {
                    listBox1.Items.Add(++ThreadCounter);
                }
            }
        }

        private void button2_Click(object sender, EventArgs e)
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

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            while (!(childProcess == null || childProcess.HasExited))
            {
                button2_Click(sender, e);
            }
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

        }
    }
}
