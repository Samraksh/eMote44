using System;
using System.Text;
using System.Threading;
using Microsoft.SPOT;

using Samraksh.eMote.Net;
using Samraksh.eMote.Net.MAC;
using Samraksh.eMote.Net.Radio;

namespace Macpipe_Test__Mel_
{
    public class Program
    {
        public static MACPipe chan1, chan2;
        public static OMAC myOMACObj;
        private static int msgID_chan2;
        private static int msgID_chan1;

        public static void Main()
        {
            try
            {
                Debug.Print("Initializing radio");

                var radioConfig = new SX1276RadioConfiguration(SX1276TxPower.Power_0dBm, SX1276Channel.Channel_915, SX1276Bandwidth.Bandwidth_500kHz, SX1276CodingRate.CodingRate_4_7, SX1276SpreadingFactor.SpreadingFactor_7);

                //configure OMAC
                myOMACObj = new OMAC(radioConfig);
                myOMACObj.NeighborLivenessDelay = 10 * 60 + 20;
                Debug.Print("NeighborLivenessDelay = " + myOMACObj.NeighborLivenessDelay);

                myOMACObj.OnNeighborChange += MacBase_OnNeighborChange;
                // myOMACObj.OnSendStatus += ReceiveSendStatus;

                Debug.Print("A #1");
                chan1 = new MACPipe(myOMACObj, PayloadType.Type01);
                chan1.OnReceive += Receive1;
                chan1.OnSendStatus += ReceiveSendStatus1;
                //Thread.Sleep(30000);
                Debug.Print("B #1");
                chan2 = new MACPipe(myOMACObj, PayloadType.Type02);
                chan2.OnReceive += Receive2;
                chan1.OnSendStatus += ReceiveSendStatus2;

             /*   Thread.Sleep(30000);
                Debug.Print("C #1");

                Thread.Sleep(60000);
                Debug.Print("D #1");

                Thread.Sleep(90000);
                Debug.Print("E #1");

                Thread.Sleep(120000);
                Debug.Print("F #1");
                */
                Debug.Print("Starting ping on macpipe #1");
                Send(chan1);

                Debug.Print("Entering seconds sleep");

                Thread.Sleep(10);

                Debug.Print("Starting ping on macpipe #2");
                Send(chan2);               
            }
            catch (Exception e)
            {
                Debug.Print("exception!: " + e.ToString());
            }

            Thread.Sleep(Timeout.Infinite);
        }

        static void MacBase_OnNeighborChange(IMAC macInstance, DateTime time)
        {
            var neighborList = MACBase.NeighborListArray();
            macInstance.NeighborList(neighborList);
            PrintNeighborList("Neighbor list CHANGE for Node [" + myOMACObj.MACRadioObj.RadioAddress + "]: ", neighborList);
        }

        private static void PrintNeighborList(string prefix, ushort[] neighborList)
        {
            PrintNumericVals(prefix, neighborList);
        }

        /// <summary>
        /// Print ushort values
        /// </summary>
        /// <param name="prefix"></param>
        /// <param name="messageEx"></param>
        public static void PrintNumericVals(string prefix, ushort[] messageEx)
        {
            var msgBldr = new StringBuilder(prefix);
            foreach (var val in messageEx)
            {
                msgBldr.Append(val + " ");
            }
            Debug.Print(msgBldr.ToString());
        }

        private static void Send(MACPipe pipe)
        {
            var neighborList = MACBase.NeighborListArray();

            // Wait until valid neighbor is picked up
            do
            {
               // Debug.Print("Waiting for neighbors...");
                pipe.NeighborList(neighborList);
            }
            while (neighborList[0] == 0);

            foreach (var theNeighbor in neighborList)
            {
                byte[] toSendByte = new byte[100];
                Debug.Print("Sending message to " + theNeighbor);
                pipe.Send(theNeighbor, toSendByte, 0, (ushort)toSendByte.Length);
            }
        }

        private static void Receive2(IMAC macInstance, DateTime time, Packet receivedPacket)
        {
            Debug.Print("---------------------------");

            if (receivedPacket == null)
            {
                Debug.Print("null");
                return;
            }

            Debug.Print("Received message " + (msgID_chan2++) + "from " + receivedPacket.Src + " on pipe 2");

            // Wait 10 seconds before sending pong back
            Thread.Sleep(20000);

            Debug.Print("Sensing pong back to " + receivedPacket.Src + " on pipe 2");

            chan2.Send(receivedPacket.Src, receivedPacket.Payload, 0, (ushort)receivedPacket.Payload.Length);
        }

        private static void Receive1(IMAC macInstance, DateTime time, Packet receivedPacket)
        {
            Debug.Print("---------------------------");

            if (receivedPacket == null)
            {
                Debug.Print("null");
                return;
            }

            Debug.Print("Received message " + (msgID_chan1++) + "from " + receivedPacket.Src + " on pipe 1");

            // Wait 10 seconds before sending pong back
            Thread.Sleep(10000);

            Debug.Print("Sensing pong back to " + receivedPacket.Src + " on pipe 1");

            chan1.Send(receivedPacket.Src, receivedPacket.Payload, 0, (ushort)receivedPacket.Payload.Length);
        }

        //Handles received messages 
        public static void ReceiveSendStatus1(IMAC macBase, DateTime time, SendPacketStatus ACKStatus, uint transmitDestination, ushort index)
        {
            Debug.Print("---------------------------");
            Debug.Print("Pipe 1 ACKStatus = " + ACKStatus + "Dest = " + transmitDestination);
            Debug.Print("---------------------------");

        }
        public static void ReceiveSendStatus2(IMAC macBase, DateTime time, SendPacketStatus ACKStatus, uint transmitDestination, ushort index)
        {
            Debug.Print("---------------------------");
            Debug.Print("Pipe 2 ACKStatus = " + ACKStatus + "Dest = " + transmitDestination);
            Debug.Print("---------------------------");

        }
    }
}
