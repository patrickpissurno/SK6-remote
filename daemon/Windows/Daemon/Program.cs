using Microsoft.Win32.TaskScheduler;
using System;
using System.IO.Ports;
using System.Threading;

namespace Daemon
{
    class Program
    {
        const int RECEIVER_HANDSHAKE_CODE = 28719;

        const int BUTTON_NEXT = 1;
        const int BUTTON_PREV = 2;
        const int BUTTON_PAUSE = 3;
        const int BUTTON_INFO = 4;
        const int BUTTON_AVSYNC = 5;

        const int BUTTON_NEXT_HOLD = 10 + BUTTON_NEXT;
        const int BUTTON_PREV_HOLD = 10 + BUTTON_PREV;
        const int BUTTON_PAUSE_HOLD = 10 + BUTTON_PAUSE;
        const int BUTTON_INFO_HOLD = 10 + BUTTON_INFO;
        const int BUTTON_AVSYNC_HOLD = 10 + BUTTON_AVSYNC;

        static void Press(ScanCodeShort key, bool shift)
        {
            INPUT[] inputs = new INPUT[shift ? 2 : 1];
            INPUT input = new INPUT();

            if (shift)
            {
                input.type = 1; // 1 = Keyboard Input
                input.U.ki.wScan = ScanCodeShort.LSHIFT;
                input.U.ki.dwFlags = KEYEVENTF.SCANCODE;
                inputs[0] = input;
            }

            input.type = 1; // 1 = Keyboard Input
            input.U.ki.wScan = key;
            input.U.ki.dwFlags = KEYEVENTF.SCANCODE;
            inputs[shift ? 1 : 0] = input;

            Input.SendInput((uint)inputs.Length, inputs, INPUT.Size);

            if (shift)
            {
                inputs = new INPUT[1];
                input.type = 1; // 1 = Keyboard Input
                input.U.ki.wScan = ScanCodeShort.LSHIFT;
                input.U.ki.dwFlags = KEYEVENTF.SCANCODE | KEYEVENTF.KEYUP;
                inputs[0] = input;
                Input.SendInput((uint)inputs.Length, inputs, INPUT.Size);
            }

            Thread.Sleep(1);
        }

        static void Exit(int status)
        {
            if(status != 0)
            {
                using (TaskService ts = new TaskService())
                {
                    var task = ts.FindTask(ProjectInstaller.TASK_NAME);
                    if (task != null)
                        task.Run();
                }
            }
            Environment.Exit(status);
        }

        static void Main(string[] args)
        {
            try
            {
                const int bufferMax = 256;
                byte[] buf = new byte[bufferMax];

                Console.WriteLine("Daemon Started");

                //wait a bit of time for all components to initialize before trying to connect
                Thread.Sleep(10000);

                Console.WriteLine("Locating the Receiver device");

                string[] ports;
                int portsLength;
                SerialPort port = null;
                do
                {
                    Thread.Sleep(1000);
                    ports = SerialPort.GetPortNames();
                    portsLength = ports.Length;

                    foreach (var name in ports)
                    {
                        try
                        {
                            port = new SerialPort(name, 9600);
                            port.ReadTimeout = 2100;
                            port.DtrEnable = true;
                            port.Open();

                            if (port.ReadLine().Trim() == RECEIVER_HANDSHAKE_CODE.ToString())
                                break;

                            throw new Exception();
                        }
                        catch
                        {
                            portsLength -= 1;
                            if (port != null)
                            {
                                if (port.IsOpen)
                                    port.Close();
                                port.Dispose();
                            }
                            port = null;
                        }
                    }
                }
                while (portsLength < 1);

                if (port == null)
                {
                    Console.WriteLine("Failed to find a Receiver device");

                    Thread.Sleep(10);
                    Exit(1);
                }

                Console.WriteLine("Connected to the Receiver device");

                port.ReadTimeout = 5000;

                int btn = 0;
                while (true)
                {
                    Thread.Sleep(500);
                    for (int i = 0; i < 5; i++)
                    {
                        btn = 0;

                        try
                        {
                            btn = int.Parse(port.ReadLine().Trim());
                        }
                        catch
                        {
                            btn = 0;
                        }

                        if (btn == RECEIVER_HANDSHAKE_CODE)
                            continue;

                        switch (btn)
                        {
                            case BUTTON_NEXT:
                                Press(ScanCodeShort.RIGHT, false);
                                break;
                            case BUTTON_PREV:
                                Press(ScanCodeShort.LEFT, false);
                                break;
                            case BUTTON_PAUSE:
                                Press(ScanCodeShort.SPACE, false);
                                break;
                            case BUTTON_NEXT_HOLD:
                                Press(ScanCodeShort.KEY_N, true);
                                break;
                            case BUTTON_PREV_HOLD:
                                Press(ScanCodeShort.KEY_P, true);
                                break;
                        }

                        Thread.Sleep(5);
                    }

                    if (!port.IsOpen)
                        break;
                }

                port.Close();
                port.Dispose();

                Console.WriteLine("Disconnected from the Receiver device");
            }
            finally
            {
                Exit(1);
            }
        }
    }
}
