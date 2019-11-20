#define RF231
//#define SI4468

using System;
using System.Text;
using System.Collections;
using Microsoft.SPOT;
using Microsoft.SPOT.Hardware;
using System.Threading;

using Samraksh.eMote.Net;
using Samraksh.eMote.Net.MAC;
using Samraksh.eMote.Net.Radio;
using Samraksh.eMote.DotNow;

//1. This program initializes OMAC as the MAC protocol.
//  1a. Registers a function that tracks change in neighbor (NeighborChange) and a function to handle messages that are received.
//2. Pings are sent at pre-determined intervals.
//3. Pongs are sent back for received messages.
namespace Samraksh.eMote.Net.Mac.Send
{
    public class PingPayload
    {
        public UInt32 pingMsgId;
        public string pingMsgContent = "PING";

        public PingPayload()
        {

        }

        public byte[] ToBytes()
        {
            byte[] msg = new byte[4];
            msg[0] = (byte)((pingMsgId >> 24) & 0xFF);
            msg[1] = (byte)((pingMsgId >> 16) & 0xFF);
            msg[2] = (byte)((pingMsgId >> 8) & 0xFF);
            msg[3] = (byte)((pingMsgId) & 0xFF);

            //Convert string to byte array
            byte[] msgContent = System.Text.Encoding.UTF8.GetBytes(pingMsgContent);

            //Merge array containing msgID and array containing string into a single byte array for transmission
            byte[] merged = new byte[msg.Length + msgContent.Length];
            msg.CopyTo(merged, 0);
            msgContent.CopyTo(merged, msg.Length);

            return merged;
        }

        public PingPayload FromBytesToPingPayload(byte[] msg)
        {
            try
            {
                PingPayload pingPayload = new PingPayload();

                //Convert byte array to an integer
                pingPayload.pingMsgId = (UInt32)(msg[0] << 24);
                pingPayload.pingMsgId += (UInt32)(msg[1] << 16);
                pingPayload.pingMsgId += (UInt32)(msg[2] << 8);
                pingPayload.pingMsgId += (UInt32)(msg[3]);

                //Create a byte array to store the string
                byte[] msgContent = new byte[4];
                msgContent[0] = msg[4];
                msgContent[1] = msg[5];
                msgContent[2] = msg[6];
                msgContent[3] = msg[7];

                //Convert byte to char array
                char[] msgContentChar = System.Text.Encoding.UTF8.GetChars(msgContent);

                //Convert char array to string
                pingPayload.pingMsgContent = new string(msgContentChar);

                return pingPayload;
            }
            catch (Exception ex)
            {
                Debug.Print(ex.ToString());
                return null;
            }
        }
    }

    public class Program
    {
        const UInt32 totalPingCount = 10001;
        //const UInt16 MAX_NEIGHBORS = 12;
        const int initialDelayInMsecs = 30000;
        int dutyCyclePeriod = 20000;

        //bool startSend = false;
        UInt16 myAddress;
        Timer sendTimer;
        //NetOpStatus status;
        EmoteLCD lcd;
        static UInt32 sendMsgCounter = 1;

        PingPayload pingMsg = new PingPayload();

        //ReceiveCallBack myReceiveCB;
        //NeighborhoodChangeCallBack myNeibhborhoodCB;

        //MACConfiguration myMacConfig = new MACConfiguration();
        //Radio.RadioConfiguration myRadioConfig = new Radio.RadioConfiguration();
        //
        private void SendOnPipe(int i, IMAC mac, ushort neighbor, MACPipe chan)
        {
            pingMsg.pingMsgId = sendMsgCounter;
            byte[] msgBytes2 = pingMsg.ToBytes();
            //var msgBytes2 =
            //	("Message " + i + " from " + mac.MACRadioObj.RadioAddress + " to neighbor " + neighbor + " on pipe " +
            //	 chan.PayloadType).ToCharArray().ToByteArray();
            var netOpStatus = chan.Send(neighbor, msgBytes2, 0, (ushort)msgBytes2.Length);
            Debug.Print("*** Sent message " + sendMsgCounter.ToString() + " to neighbor " + neighbor + " on pipe " + chan.PayloadType + "; NetOpStatus: " + netOpStatus);
        }


        public void Initialize()
        {
            //Init LCD
            lcd = new EmoteLCD();
            lcd.Initialize();
            lcd.Write(LCD.CHAR_I, LCD.CHAR_n, LCD.CHAR_i, LCD.CHAR_t);

            try
            {
                OMAC myMac;
                Debug.Print("Initializing radio");
#if RF231
                var radioConfig = new RF231RadioConfiguration(RF231TxPower.Power_0Point0dBm, RF231Channel.Channel_13);
#elif SI4468
                var radioConfig = new SI4468RadioConfiguration(SI4468TxPower.Power_15Point5dBm, SI4468Channel.Channel_01);
#endif

                //configure OMAC
                myMac = new OMAC(radioConfig);
                myMac.OnReceive += Rc;
				myMac.OnSendStatus += SendStatus;
                myMac.OnNeighborChange += NeighborChange;

                myAddress = myMac.MACRadioObj.RadioAddress;
                Debug.Print("My address is: " + myAddress.ToString() + ". I am in Send mode");

                var chan1 = new MACPipe(myMac, PayloadType.Type01);
                chan1.OnReceive += Rc1;

                var chan2 = new MACPipe(myMac, PayloadType.Type02);
                chan2.OnReceive += Rc2;

                ushort[] _neighborList;
                _neighborList = MACBase.NeighborListArray();

                var rand = new Random();

                while (true)
                {
                    for (int i = 0; i < 25; i++)
                    {
                        var status = myMac.NeighborList(_neighborList);
                        foreach (var neighbor in _neighborList)
                        {
                            if (neighbor == 0) { continue; }
                            SendOnPipe(1, myMac, neighbor, chan1);
                        }
                        sendMsgCounter++;
                        var waitTime = (int)(rand.NextDouble() * 5 * 1000);
                        waitTime = System.Math.Max(waitTime, 5 * 1000);
                        Debug.Print("*** Waiting " + waitTime);
                        Thread.Sleep(waitTime);
                    }
                    Debug.Print("Disposing of mac");
                    myMac.Dispose();
                    Thread.Sleep(200000);
                    myMac = new OMAC(radioConfig);
                    myMac.OnReceive += Rc;
                    myMac.OnNeighborChange += NeighborChange;
                }

            }
            catch (Exception e)
            {
                Debug.Print("exception!: " + e.ToString());
            }


        }

		public void SendStatus(IMAC macBase, DateTime time, SendPacketStatus ACKStatus, uint transmitDestination, ushort index)
		{
			if (ACKStatus == SendPacketStatus.SendInitiated)
				Debug.Print("Packet to " + transmitDestination.ToString() + " started\r\n" );
			else if (ACKStatus == SendPacketStatus.SendACKed)
				Debug.Print("Packet to " + transmitDestination.ToString() + "  ACK\r\n");
			else if (ACKStatus == SendPacketStatus.SendNACKed)
				Debug.Print("Packet to " + transmitDestination.ToString() + "  NACK\r\n");
			else if (ACKStatus == SendPacketStatus.SendFailed)
				Debug.Print("Packet to " + transmitDestination.ToString() + "  failed\r\n");
		}
        //Keeps track of change in neighborhood
        public void NeighborChange(IMAC macBase, DateTime time)
        {
            //Debug.Print("Count of neighbors " + countOfNeighbors.ToString());
        }

        //Starts a timer 
        public void Start()
        {
            Debug.Print("Waiting to start test");
            Thread.Sleep(initialDelayInMsecs);
            Debug.Print("Starting timer...");
            TimerCallback timerCB = new TimerCallback(sendTimerCallback);
            sendTimer = new Timer(timerCB, null, 0, dutyCyclePeriod);
            Debug.Print("Timer initialization done");
        }

        //Calls ping at regular intervals
        void sendTimerCallback(Object obj)
        {
            SendPing();
        }

        public void SendPing()
        {
            /*try
            {
                bool sendFlag = false;
                ushort[] _neighborList;

				_neighborList = MACBase.NeighborListArray();
				foreach (var neighbor in _neighborList){
                        Debug.Print("count of neighbors " + _neighborList.Length);
                        startSend = true; sendFlag = true;
                        pingMsg.pingMsgId = sendMsgCounter;
                        byte[] msg = pingMsg.ToBytes();
                        Debug.Print("Sending to neighbor " + neighbor.ToString() + " ping msgID " + sendMsgCounter);
                        status = myOMACObj.Send(neighbor, PayloadType.MFM_Data, msg, 0, (ushort)msg.Length);
                        if (status != NetOpStatus.S_Success)
                        {
                            Debug.Print("Send failed. Ping msgID " + sendMsgCounter.ToString());
                        }
                }
                if (sendFlag == false && startSend == true)
                {
                    Debug.Print("Ping failed. All neighbors dropped out");
                }
                //Increment msgCounter only if there is a send
                if (sendFlag == true)
                {
                    sendMsgCounter++;
                }

                if (sendMsgCounter < 10)
                {
                    lcd.Write(LCD.CHAR_S, LCD.CHAR_S, LCD.CHAR_S, (LCD)sendMsgCounter);
                }
                else if (sendMsgCounter < 100)
                {
                    UInt16 tenthPlace = (UInt16)(sendMsgCounter / 10);
                    UInt16 unitPlace = (UInt16)(sendMsgCounter % 10);
                    lcd.Write(LCD.CHAR_S, LCD.CHAR_S, (LCD)tenthPlace, (LCD)unitPlace);
                }
                else if (sendMsgCounter < 1000)
                {
                    UInt16 hundredthPlace = (UInt16)(sendMsgCounter / 100);
                    UInt16 remainder = (UInt16)(sendMsgCounter % 100);
                    UInt16 tenthPlace = (UInt16)(remainder / 10);
                    UInt16 unitPlace = (UInt16)(remainder % 10);
                    lcd.Write(LCD.CHAR_S, (LCD)hundredthPlace, (LCD)tenthPlace, (LCD)unitPlace);
                }
                else if (sendMsgCounter < 10000)
                {
                    UInt16 thousandthPlace = (UInt16)(sendMsgCounter / 1000);
                    UInt16 remainder = (UInt16)(sendMsgCounter % 1000);
                    UInt16 hundredthPlace = (UInt16)(remainder / 100);
                    remainder = (UInt16)(remainder % 100);
                    UInt16 tenthPlace = (UInt16)(remainder / 10);
                    UInt16 unitPlace = (UInt16)(remainder % 10);
                    lcd.Write((LCD)thousandthPlace, (LCD)hundredthPlace, (LCD)tenthPlace, (LCD)unitPlace);
                }

                if (sendMsgCounter == totalPingCount)
                {
                    sendMsgCounter--;
                    ShowStatistics();
                }
            }
            catch (Exception ex)
            {
                Debug.Print("SendPing: " + ex.ToString());
            }*/
        }

        private static void Rc(IMAC macBase, DateTime timeReceived, Packet receivedPacket)
        {
            RcCommon(macBase, timeReceived, receivedPacket);
        }
        private static void Rc1(IMAC macBase, DateTime timeReceived, Packet receivedPacket)
        {
            RcCommon(macBase, timeReceived, receivedPacket);
        }
        private static void Rc2(IMAC macBase, DateTime timeReceived, Packet receivedPacket)
        {
            RcCommon(macBase, timeReceived, receivedPacket);
        }

        private static void RcCommon(IMAC macBase, DateTime timeReceived, Packet receivedPacket)
        {
            var macPipe = (MACPipe)macBase;
            var plType = macPipe.PayloadType;
            Debug.Print("*** Packet received\n");
            //var packet = macBase.NextPacket();
            //Debug.Print("\t1");
            if (receivedPacket == null) { return; }
            //Debug.Print("\t2");
            var payloadBytes = receivedPacket.Payload;
            //Debug.Print("\t3");
            //var payloadChars = payloadBytes.ToCharArray();
            //Debug.Print("\t4");
            //var payloadStr = new string(payloadChars);
            //Debug.Print("\t5");
            var payloadStr = payloadBytes[0].ToString() + payloadBytes[1].ToString() + payloadBytes[2].ToString() + payloadBytes[3].ToString();

            Debug.Print("*** Payload type " + plType + ", Received " + timeReceived.ToString("G") + ", <" + payloadStr + ">");
        }


        //Show statistics
        void ShowStatistics()
        {
            Debug.Print("==============STATS================");
            Debug.Print("total msgs sent " + sendMsgCounter);
            Debug.Print("==================================");
            Thread.Sleep(Timeout.Infinite);
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



