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
    public class PingPayload_long
    {
        public UInt32 pingMsgId;
        public string pingMsgContent = "PING";

        public PingPayload_long()
        {

        }

        public byte[] ToBytes()
        {
            byte[] msg = new byte[92];
            msg[0] = (byte)((pingMsgId >> 24) & 0xFF);
            msg[1] = (byte)((pingMsgId >> 16) & 0xFF);
            msg[2] = (byte)((pingMsgId >> 8) & 0xFF);
            msg[3] = (byte)((pingMsgId) & 0xFF);
            for (int i = 4; i < 92; i++)
            {
                msg[i] = (byte)i;
            }

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
#if RF231
        const int initialDelayInMsecs = 30000;
#elif SI4468
        const int initialDelayInMsecs = 75000;
#endif
        int dutyCyclePeriod = 20000;

        //bool startSend = false;
        UInt16 myAddress;
        UInt16 chan1Address;
        UInt16 chan2Address;
        MACPipe chan1, chan2;
        Timer sendTimer;
        //NetOpStatus status;
        EmoteLCD lcd;
        static UInt32 sendMsgCounter = 1;

        PingPayload pingMsg = new PingPayload();
        PingPayload_long pingMsg_long = new PingPayload_long();

        private void SendOnPipe(int i, IMAC mac, ushort neighbor, MACPipe chan)
        {
            if (sendMsgCounter % 2 == 0)
            {
                pingMsg.pingMsgId = sendMsgCounter;
                byte[] msgBytes2 = pingMsg.ToBytes();
                //var msgBytes2 =
                //	("Message " + i + " from " + mac.MACRadioObj.RadioAddress + " to neighbor " + neighbor + " on pipe " +
                //	 chan.PayloadType).ToCharArray().ToByteArray();
                var netOpStatus = chan.Send(neighbor, msgBytes2, 0, (ushort)msgBytes2.Length);
                Debug.Print("*** Sent message " + sendMsgCounter.ToString() + " of length " + (ushort)msgBytes2.Length + " to neighbor " + neighbor + " on pipe " + chan.PayloadType + "; NetOpStatus: " + netOpStatus);
            }
            else
            {
                pingMsg_long.pingMsgId = sendMsgCounter;
                byte[] msgBytes2 = pingMsg_long.ToBytes();
                //var msgBytes2 =
                //	("Message " + i + " from " + mac.MACRadioObj.RadioAddress + " to neighbor " + neighbor + " on pipe " +
                //	 chan.PayloadType).ToCharArray().ToByteArray();
                var netOpStatus = chan.Send(neighbor, msgBytes2, 0, (ushort)msgBytes2.Length);
                Debug.Print("*** Sent long message " + sendMsgCounter.ToString() + " of length " + (ushort)msgBytes2.Length + " to neighbor " + neighbor + " on pipe " + chan.PayloadType + "; NetOpStatus: " + netOpStatus);
            }

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
                var radioConfig = new RF231RadioConfiguration(RF231TxPower.Power_3dBm, RF231Channel.Channel_13);
#elif SI4468
                var radioConfig = new SI4468RadioConfiguration(SI4468TxPower.Power_1Point1dBm, SI4468Channel.Channel_01);
#endif

                //configure OMAC
                myMac = new OMAC(radioConfig);
                myMac.OnReceive += Rc;
                myMac.OnNeighborChange += NeighborChange;


                myAddress = myMac.MACRadioObj.RadioAddress;
                Debug.Print("CheckAutoSupport1 Test 1  My address is: " + myAddress.ToString() + ". I am in Send mode");

                chan1 = new MACPipe(myMac, PayloadType.Type01);
                chan1.OnReceive += Rc1;
                chan1.OnSendStatus += ReceiveSendStatus1;

                chan2 = new MACPipe(myMac, PayloadType.Type02);
                chan2.OnReceive += Rc2;
                chan2.OnSendStatus += ReceiveSendStatus2;

                ushort[] _neighborList;
                _neighborList = MACBase.NeighborListArray();

                var rand = new Random();

                while (true)
                {
                    var status = myMac.NeighborList(_neighborList);
                    var i = 1;
                    foreach (var neighbor in _neighborList)
                    {
                        if (neighbor == 0) { continue; }
                        if (i == 1)
                        {
                            SendOnPipe(1, myMac, neighbor, chan1);
                            i = 2;
                            chan1Address = neighbor;
                        }
                        else
                        {
                            SendOnPipe(1, myMac, neighbor, chan2);
                            i = 1;
                            chan2Address = neighbor;
                        }
                    }
                    sendMsgCounter++;
                    var waitTime = (int)(rand.NextDouble() * 30 * 1000);
                    waitTime = System.Math.Max(waitTime, 20 * 1000);
                    //Debug.Print("*** Waiting " + waitTime);
                    Thread.Sleep(waitTime);
                }

            }
            catch (Exception e)
            {
                Debug.Print("exception!: " + e.ToString());
            }
        }

        //Keeps track of change in neighborhood
        public void NeighborChange(IMAC macBase, DateTime time)
        {
            Debug.Print("---- neighbor change ----\r\n");

        }

        //Handles received messages 
        public void ReceiveSendStatus1(IMAC macBase, DateTime time, SendPacketStatus ACKStatus, uint transmitDestination, ushort index)
        {
            var pipe = macBase as MACPipe;
            Debug.Print("---------------------------");
            Debug.Print("ReceiveSendStatus1 ACKStatus = " + ACKStatus + " Dest = " + transmitDestination);
            /*if (ACKStatus != SendPacketStatus.SendACKed)
            {
                byte[] msg = new byte[96];
                if (pipe.GetMsgWithMsgID(ref msg, index) == Samraksh.eMote.Net.DeviceStatus.Success)
                {
                    //pingMsg.pingMsgId = sendMsgCounter;
                    //byte[] msgBytes2 = pingMsg.ToBytes();
                    var netOpStatus = chan1.Send(chan1Address, msg, 0, (ushort)msg.Length);
                    Debug.Print("~~~~ resent to " + chan1Address.ToString() + " ~~~~");
                }

            }*/

        }
        public void ReceiveSendStatus2(IMAC macBase, DateTime time, SendPacketStatus ACKStatus, uint transmitDestination, ushort index)
        {
            var pipe = macBase as MACPipe;
            Debug.Print("---------------------------");
            Debug.Print("ReceiveSendStatus2 ACKStatus = " + ACKStatus + " Dest = " + transmitDestination);
            /*if (ACKStatus != SendPacketStatus.SendACKed)
            {
                byte[] msg = new byte[96];
                if (pipe.GetMsgWithMsgID(ref msg, index) == Samraksh.eMote.Net.DeviceStatus.Success)
                {
                    //pingMsg.pingMsgId = sendMsgCounter;
                    //byte[] msgBytes2 = pingMsg.ToBytes();
                    var netOpStatus = chan2.Send(chan2Address, msg, 0, (ushort)msg.Length);
                    Debug.Print("~~~~ resent to " + chan2Address.ToString() + " ~~~~");
                }
            }*/
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
                UInt16[] neighborList = OMAC.NeighborListArray();
                DeviceStatus dsStatus = myOMACObj.NeighborList(neighborList);

                for (int j = 0; j < MAX_NEIGHBORS; j++)
                {
                    if (neighborList[j] != 0)
                    {
                        //Debug.Print("count of neighbors " + neighborList.Length);
                        startSend = true; sendFlag = true;
                        pingMsg.pingMsgId = sendMsgCounter;
                        byte[] msg = pingMsg.ToBytes();
                        Debug.Print("Sending to neighbor " + neighborList[j] + " ping msgID " + sendMsgCounter);
                        status = myOMACObj.Send(neighborList[j], msg, 0, (ushort)msg.Length);
                        //Debug.Print("Sending to neighbor " + 6846 + " ping msgID " + sendMsgCounter);
                        //status = myOMACObj.Send(6846, PayloadType.MFM_Data, msg, 0, (ushort)msg.Length);
                        if (status != NetOpStatus.S_Success)
                        {
                            Debug.Print("Send failed. Ping msgID " + sendMsgCounter.ToString());
                        }
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

        private static void Rc(IMAC mac, DateTime timeReceived, Packet receivedPacket)
        {
            RcCommon(mac, timeReceived, receivedPacket);
        }
        private static void Rc1(IMAC mac, DateTime timeReceived, Packet receivedPacket)
        {
            RcCommon(mac, timeReceived, receivedPacket);
        }
        private static void Rc2(IMAC mac, DateTime timeReceived, Packet receivedPacket)
        {
            RcCommon(mac, timeReceived, receivedPacket);
        }

        private static void RcCommon(IMAC mac, DateTime timeReceived, Packet receivedPacket)
        {
            var macPipe = (MACPipe)mac;
            var plType = macPipe.PayloadType;
            //Debug.Print("*** Packet received\n");
            //var packet = mac.NextPacket();
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




