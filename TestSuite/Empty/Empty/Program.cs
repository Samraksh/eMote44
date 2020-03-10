using System;
using Microsoft.SPOT;
using Microsoft.SPOT.Hardware;
//using Samraksh.SPOT.Hardware.EmoteDotNow;
using System.Threading;
//using ParameterClass;

namespace TestSuite
{
    public class Program
    {
        private static Microsoft.SPOT.Hardware.OutputPort testPort_PA0 = new Microsoft.SPOT.Hardware.OutputPort((Cpu.Pin)0, true);
        public static void Main()
        {
            Debug.Print("--- application is running ---");
            //while (true)
            //{
            //    System.Threading.Thread.Sleep(1000);
            // }
            while (true)
            {
                Debug.Print("C# app");
                testPort_PA0.Write(true);
                System.Threading.Thread.Sleep(1000);
                testPort_PA0.Write(false);
                System.Threading.Thread.Sleep(1000);
            }
        }
    }
}

