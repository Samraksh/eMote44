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
        // should give a pass fail after 19.5 minutes
		const int testCount = 2925;
        UInt16 myAddress;
        UInt16 mySeqNo = 1;
		UInt16 errorCnt = 0;
		UInt16 receivePackets = 0;
        UInt16 lastRxSeqNo = 0;
        ushort[] rxBuffer = new ushort[testCount];
        Timer sendTimer;
        EmoteLCD lcd;
        PingMsg sendMsg = new PingMsg();
        Random rand = new Random();
        CSMA myCSMA;

        void Initialize()
        {
            Debug.Print("Initializing:  EmotePingwLCD");
            Thread.Sleep(1000);
            lcd = new EmoteLCD();
            lcd.Initialize();
            lcd.Write(LCD.CHAR_I, LCD.CHAR_N, LCD.CHAR_I, LCD.CHAR_7);

            try
            {
                Debug.Print("Initializing radio");
#if RF231
                var radioConfig = new RF231RadioConfiguration(RF231TxPower.Power_0Point0dBm, RF231Channel.Channel_13);
#elif SI4468
                var radioConfig = new SI4468RadioConfiguration(SI4468TxPower.Power_15Point5dBm, SI4468Channel.Channel_01);
#endif
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
            myAddress = myCSMA.MACRadioObj.RadioAddress;
            Debug.Print("My default address is :  " + myAddress.ToString());
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
			if ((receivePackets%100)==1){
				Debug.Print(receivePackets.ToString());
			}
			if (receivePackets >= ((int)(testCount * 0.98))){
				Debug.Print("result = PASS");
				Debug.Print("accuracy = null");
				Debug.Print("resultParameter1 = " + receivePackets.ToString());
				Debug.Print("resultParameter2 = " + testCount.ToString());
				Debug.Print("resultParameter3 = null");
				Debug.Print("resultParameter4 = null");
				Debug.Print("resultParameter5 = null"); 			
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
            try
            {
                if (size == PingMsg.Size())
                {

                    Debug.Print("MSG: " + msg[0].ToString() + " " + msg[1].ToString() + " " + msg[2].ToString() + " " + msg[3].ToString() + " " + msg[4].ToString() + " " + msg[5].ToString());
                    PingMsg rcvMsg = new PingMsg(msg, size);

                    //if (rcvMsg.Response)
                    //{
                        if (receivePackets < testCount)
                        {
                            //This is a response to my message						
                            rxBuffer[receivePackets] = rcvMsg.MsgID;
                        }
                        receivePackets++;
						if ( ((UInt16)rcvMsg.MsgID) != lastRxSeqNo + 1){
							errorCnt++;
							Debug.Print("***** Missing seq no: " + (lastRxSeqNo + 1).ToString() + " *****");
						}
						lastRxSeqNo = (UInt16)rcvMsg.MsgID;
						//if ( (receivePackets % 1000) == 0)
 	                       Debug.Print("Rx: " + rcvMsg.Src.ToString() + " seq: " + rcvMsg.MsgID.ToString() + " err: " + errorCnt.ToString());						
                        lcd.Write(LCD.CHAR_P, LCD.CHAR_P, LCD.CHAR_P, LCD.CHAR_P);
                    //}
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

