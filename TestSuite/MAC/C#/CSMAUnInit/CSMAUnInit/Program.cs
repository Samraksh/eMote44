#define RF231
//#define SI4468

using System;
using Microsoft.SPOT;
using Microsoft.SPOT.Hardware;
using System.Threading;

using Samraksh.eMote.Net;
using Samraksh.eMote.Net.Radio;
using Samraksh.eMote.Net.MAC;
using Samraksh.eMote.DotNow;

namespace Samraksh.eMote.Net.Mac.Ping
{
    public class DummyMsg
    {
        public byte[] data = new byte[10];
        public DummyMsg()
        {
            data[0] = 1; data[2] = 2; data[4] = 4; data[6] = 6; data[8] = 8;
        }
    }


    public class PingMsg
    {
        public bool Response;
        public ushort MsgID;
        public UInt16 Src;
        public UInt16 dummySrc;

        public PingMsg()
        {
        }
        public static int Size()
        {
            return 7;
        }
        public PingMsg(byte[] rcv_msg, ushort size)
        {
            Response = rcv_msg[0] == 0 ? false : true;
            MsgID = (UInt16)(rcv_msg[1] << 8);
            MsgID += (UInt16)rcv_msg[2];
            Src = (UInt16)(rcv_msg[3] << 8);
            Src += (UInt16)rcv_msg[4];
            dummySrc = (UInt16)(0xefef);

        }

        public byte[] ToBytes()
        {
            byte[] b_msg = new byte[7];
            b_msg[0] = Response ? (byte)1 : (byte)0;
            b_msg[1] = (byte)((MsgID >> 8) & 0xFF);
            b_msg[2] = (byte)(MsgID & 0xFF);
            b_msg[3] = (byte)((Src >> 8) & 0xFF);
            b_msg[4] = (byte)(Src & 0xFF);
            b_msg[5] = (byte)(0xef);
            b_msg[6] = (byte)(0xef);
            return b_msg;
        }
    }

    public class Program
    {
        NetOpStatus status;
        const int firstPos = 20;
        const int testCount = 150;
        UInt16 myAddress;
        UInt16 mySeqNo = 1;
		UInt16 receivePackets = 0;
        UInt16 lastRxSeqNo = 0;
        ushort[] rxBuffer = new ushort[testCount];
        Timer sendTimer;
        EmoteLCD lcd;
        PingMsg sendMsg = new PingMsg();
        //DummyMsg myDummy = new DummyMsg();
        Random rand = new Random();
        //Radio.Radio_802_15_4 my_15_4 = new Radio.Radio_802_15_4();
        //Radio.RadioConfiguration radioConfig = new Radio.RadioConfiguration();
        //int myRadioID;

        CSMA myCSMA;
        //ReceiveCallBack myReceiveCB;
        //NeighborhoodChangeCallBack myNeighborCB;

        //Mac.MacConfiguration macConfig = new MacConfiguration();
        //MACConfiguration macConfig = new MACConfiguration();

        void Initialize()
        {
            Debug.Print("Initializing:  EmotePingwLCD");
            Thread.Sleep(1000);
            lcd = new EmoteLCD();
            lcd.Initialize();
            lcd.Write(LCD.CHAR_I, LCD.CHAR_N, LCD.CHAR_I, LCD.CHAR_7);
            Debug.Print("Initializing radio");
#if RF231
            var radioConfig = new RF231RadioConfiguration(RF231TxPower.Power_0Point0dBm, RF231Channel.Channel_13);
#elif SI4468
            var radioConfig = new SI4468RadioConfiguration(SI4468TxPower.Power_15Point5dBm, SI4468Channel.Channel_01);
#endif

            try
            {
                Debug.Print("Configuring:  CSMA...");
                myCSMA = new CSMA(radioConfig);
                myCSMA.OnReceive += Receive;
                myCSMA.OnNeighborChange += NeighborChange;
            }
            catch (Exception e)
            {
                Debug.Print(e.ToString());
            }

            Debug.Print("CSMA Init done.");
			Debug.Print("Uninitializing CSMA.");
			myCSMA.Dispose();
			Debug.Print("Initializing CSMA again.");

			try
            {
                myCSMA = new CSMA(radioConfig);
				myCSMA.OnReceive += Receive;
                myCSMA.OnNeighborChange += NeighborChange;
            }
            catch (Exception e)
            {
                Debug.Print(e.ToString());
            }
			Debug.Print("2nd CSMA Init done.");

            myAddress = myCSMA.MACRadioObj.RadioAddress;
            Debug.Print("My default address is :  " + myAddress.ToString());

            /*myCSMA.SetAddress(52);
            myAddress = myCSMA.GetAddress();
            Debug.Print("My New address is :  " + myAddress.ToString());
             */

        }
        void Start()
        {
            Debug.Print("Starting timer...");
            sendTimer = new Timer(new TimerCallback(sendTimerCallback), null, 0, 400);
            Debug.Print("Timer init done.");
        }

        void sendTimerCallback(Object o)
        {
            // We receieved enough data....looking to see if we received all packets (even in best case scenario we could have a few errors)
            // we wait a bit longer just so the other side will also receive enough packets
            if (receivePackets >= (testCount + 20))
            {
                int i, j;
                bool found = false;
                int errors = 0;
                // searching for testCount-20 numbers starting with the first number we received
                ushort comparisonValue = rxBuffer[firstPos];
                for (i = firstPos; i < testCount - 20; i++)
                {
                    found = false;
                    for (j = firstPos; j < testCount; j++)
                    {
                        if (rxBuffer[j] == comparisonValue + 1)
                        {
                            found = true;
                        }
                    }
                    if (found != true)
                    {
                        Debug.Print("couldn't find " + (comparisonValue + 1).ToString() + " @ " + i.ToString() + " found " + rxBuffer[i].ToString());
                        errors++;
                    }
                    comparisonValue = rxBuffer[i];
                }
                if ((receivePackets % 5) == 0)
                {
                    if (errors < 6)
                    {
                        Debug.Print("result = PASS");
                        Debug.Print("accuracy = " + errors.ToString());
                        Debug.Print("resultParameter1 = " + rxBuffer[firstPos].ToString());
                        Debug.Print("resultParameter2 = " + rxBuffer[testCount - 1].ToString());
                        Debug.Print("resultParameter3 = " + receivePackets.ToString());
                        Debug.Print("resultParameter4 = null");
                        Debug.Print("resultParameter5 = null");
                    }
                    else
                    {
                        Debug.Print("result = FAIL");
                        Debug.Print("accuracy = " + errors.ToString());
                        Debug.Print("resultParameter1 = " + rxBuffer[firstPos].ToString());
                        Debug.Print("resultParameter2 = " + rxBuffer[testCount - 1].ToString());
                        Debug.Print("resultParameter3 = " + receivePackets.ToString());
                        Debug.Print("resultParameter4 = null");
                        Debug.Print("resultParameter5 = null");
                    }
                }
            }
            try
            {
                Send_Ping(sendMsg);
            }
            catch (Exception e)
            {
                Debug.Print(e.ToString());
            }

        }

        void NeighborChange(IMAC macBase, DateTime date)
        {
			//Debug.Print("neighbor count: " + noOfNeighbors.ToString());
        }

        void Receive(IMAC macBase, DateTime date, Packet receivedPacket)
        {
            /*if (myCSMA.PendingReceivePacketCount() == 0)
            {
                Debug.Print("no packets");
                return;
            }

            //while (myCSMA.GetPendingPacketCount() > 0) {
            Packet receivedPacket = myCSMA.NextPacket();*/
            if (receivedPacket == null)
            {
                Debug.Print("null");
                return;
            }

            byte[] rcvPayload = receivedPacket.Payload;
            HandleMessage(rcvPayload, (UInt16)receivedPacket.Size, receivedPacket.Src, receivedPacket.IsUnicast, receivedPacket.RSSI, receivedPacket.LQI);
            //}
            /*try{
            // Check if there's at least one packet
            if (myCSMA.GetPendingPacketCount() < 1) {
                Debug.Print("no packets");
                return;
            }
				
            Message rcvMsg = myCSMA.GetNextPacket();
            if (rcvMsg == null) {
                Debug.Print("null");
                return;
            }
			
            byte[] rcvPayload = rcvMsg.GetMessage();
            HandleMessage(rcvPayload, (UInt16)rcvMsg.Size, rcvMsg.Src, rcvMsg.Unicast, rcvMsg.RSSI, rcvMsg.LQI);
            }
             catch (Exception e)
            {
                Debug.Print("Receive:" + e.ToString());
            }*/
        }



        void HandleMessage(byte[] msg, UInt16 size, UInt16 src, bool unicast, byte rssi, byte lqi)
        {
            //Debug.Print("HandleMessage; size is " + size);
            try
            {
                /*if (unicast)
                {
                    Debug.Print("Got a Unicast message from src: " + src.ToString() + ", size: " + size.ToString() + ", rssi: " + rssi.ToString() + ", lqi: " + lqi.ToString());
                }
                else
                {
                    Debug.Print("Got a broadcast message from src: " + src.ToString() + ", size: " + size.ToString() + ", rssi: " + rssi.ToString() + ", lqi: " + lqi.ToString());
                }*/
                if (size == PingMsg.Size())
                {

                    //Debug.Print("MSG: " + msg[0].ToString() + " " + msg[1].ToString() + " " + msg[2].ToString() + " " + msg[3].ToString() + " " + msg[4].ToString() + " " + msg[5].ToString());
                    PingMsg rcvMsg = new PingMsg(msg, size);

                    if (rcvMsg.Response)
                    {
                        if (receivePackets < testCount)
                        {
                            //This is a response to my message						
                            rxBuffer[receivePackets] = rcvMsg.MsgID;
                        }
                        receivePackets++;
                        //Debug.Print("Received response from: " + rcvMsg.Src.ToString() + " for seq no: " + rcvMsg.MsgID.ToString());						
                        lcd.Write(LCD.CHAR_P, LCD.CHAR_P, LCD.CHAR_P, LCD.CHAR_P);
                    }
                    else
                    {
                        //if ( ((UInt16)rcvMsg.MsgID) != lastRxSeqNo + 1)
                        //	Debug.Print("***** Missing seq no: " + (lastRxSeqNo + 1).ToString() + " *****");
                        //Debug.Print("Sending a Pong to SRC: " + rcvMsg.Src.ToString() + " for seq no: " + rcvMsg.MsgID.ToString());
                        lastRxSeqNo = (UInt16)rcvMsg.MsgID;
                        lcd.Write(LCD.CHAR_R, LCD.CHAR_R, LCD.CHAR_R, LCD.CHAR_R);
                        Send_Pong(rcvMsg);
                    }
                    //Debug.GC(true);
                }
                else
                {
                    Debug.Print("not proper size with possible ID of: " + ((UInt16)(msg[1] << 8)).ToString());
                }
            }
            catch (Exception e)
            {
                Debug.Print("HandleMessage:" + e.ToString());
            }
        }

        void Send_Pong(PingMsg ping)
        {
            try
            {
                UInt16 sender = ping.Src;
                ping.Response = true;

                ping.Src = myAddress;

                byte[] payload = ping.ToBytes();
                status = myCSMA.Send(sender, PayloadType.MFM_Data, payload, 0, (ushort)payload.Length);
                if (status != NetOpStatus.S_Success)
                {
                    Debug.Print("Failed to send: " + ping.MsgID.ToString());
                }
            }
            catch (Exception e)
            {
                Debug.Print("Send_Pong:" + e.ToString());
            }
        }

        void Send_Ping(PingMsg ping)
        {
            try
            {
                //UInt16 sender = ping.Src;
                //Debug.GC(true);
                ping.Response = false;
                ping.MsgID = mySeqNo++;
                ping.Src = myAddress;


                byte[] payload = ping.ToBytes();
                //Debug.Print("Send_Ping sending " + ping.MsgID.ToString());
                status = myCSMA.Send((UInt16)MAC.AddressType.Broadcast, PayloadType.MFM_Data, payload, 0, (ushort)payload.Length);
                if (status != NetOpStatus.S_Success)
                {
                    Debug.Print("Failed to send: " + ping.MsgID.ToString());
                }

                int char0 = (mySeqNo % 10) + (int)LCD.CHAR_0;
                lcd.Write(LCD.CHAR_S, LCD.CHAR_S, LCD.CHAR_S, (LCD)char0);
            }
            catch (Exception e)
            {
                Debug.Print("Send_Ping:" + e.ToString());
            }
        }

        public static void Main()
        {
            Program p = new Program();
            p.Initialize();
            p.Start();
            Thread.Sleep(Timeout.Infinite);
        }
    }
}


