#define RF231
//#define SI4468

using System;
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
namespace Samraksh.eMote.Net.Mac.Ping
{
    public class PingPayload
    {
        public UInt32 pingMsgId;
        public string pingMsgContent = "PING";
        public UInt16 pingSenderAddress;

        public PingPayload() { }

        public byte[] ToBytes()
        {
            byte[] msgId = new byte[4];
            msgId[0] = (byte)((pingMsgId >> 24) & 0xFF);
            msgId[1] = (byte)((pingMsgId >> 16) & 0xFF);
            msgId[2] = (byte)((pingMsgId >> 8) & 0xFF);
            msgId[3] = (byte)((pingMsgId) & 0xFF);

            //Convert string to byte array
            byte[] msgContent = System.Text.Encoding.UTF8.GetBytes(pingMsgContent);

            byte[] senderAddress = new byte[2];
            senderAddress[0] = (byte)((pingSenderAddress >> 8) & 0xFF);
            senderAddress[1] = (byte)((pingSenderAddress) & 0xFF);

            //Merge array containing msgID and array containing string into a single byte array for transmission
            byte[] merged = new byte[msgId.Length + msgContent.Length + senderAddress.Length];
            msgId.CopyTo(merged, 0);
            msgContent.CopyTo(merged, msgId.Length);
            senderAddress.CopyTo(merged, msgId.Length + msgContent.Length);

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

                //Convert byte array to ping sender's address
                pingPayload.pingSenderAddress += (UInt16)(msg[8] << 8);
                pingPayload.pingSenderAddress += (UInt16)(msg[9]);

                return pingPayload;
            }
            catch (Exception ex)
            {
                Debug.Print(ex.ToString());
                return null;
            }
        }
    }

    public class PongPayload
    {
        public UInt32 pongMsgId;
        public string pongMsgContent = "PONG";
        public UInt16 pongSenderAddress;
        public UInt16 pongReceiverAddress;

        public PongPayload() { }

        public byte[] ToBytes()
        {
            byte[] msgId = new byte[4];
            msgId[0] = (byte)((pongMsgId >> 24) & 0xFF);
            msgId[1] = (byte)((pongMsgId >> 16) & 0xFF);
            msgId[2] = (byte)((pongMsgId >> 8) & 0xFF);
            msgId[3] = (byte)((pongMsgId) & 0xFF);

            //Convert string to byte array
            byte[] msgContent = System.Text.Encoding.UTF8.GetBytes(pongMsgContent);

            byte[] senderAddress = new byte[2];
            senderAddress[0] = (byte)((pongSenderAddress >> 8) & 0xFF);
            senderAddress[1] = (byte)((pongSenderAddress) & 0xFF);

            byte[] receiverAddress = new byte[2];
            receiverAddress[0] = (byte)((pongReceiverAddress >> 8) & 0xFF);
            receiverAddress[1] = (byte)((pongReceiverAddress) & 0xFF);

            //Merge array containing msgID and array containing string into a single byte array for transmission
            byte[] merged = new byte[msgId.Length + msgContent.Length + senderAddress.Length + receiverAddress.Length];
            msgId.CopyTo(merged, 0);
            msgContent.CopyTo(merged, msgId.Length);
            senderAddress.CopyTo(merged, msgId.Length + msgContent.Length);
            receiverAddress.CopyTo(merged, msgId.Length + msgContent.Length + senderAddress.Length);

            return merged;
        }

        public PongPayload FromBytesToPongPayload(byte[] msg)
        {
            try
            {
                PongPayload pongPayload = new PongPayload();

                //Convert byte array to an integer
                pongPayload.pongMsgId = (UInt32)(msg[0] << 24);
                pongPayload.pongMsgId += (UInt32)(msg[1] << 16);
                pongPayload.pongMsgId += (UInt32)(msg[2] << 8);
                pongPayload.pongMsgId += (UInt32)(msg[3]);

                //Create a byte array to store the string
                byte[] msgContent = new byte[4];
                msgContent[0] = msg[4];
                msgContent[1] = msg[5];
                msgContent[2] = msg[6];
                msgContent[3] = msg[7];

                //Convert byte to char array
                char[] msgContentChar = System.Text.Encoding.UTF8.GetChars(msgContent);

                //Convert char array to string
                pongPayload.pongMsgContent = new string(msgContentChar);

                //Convert byte array to ping sender's address
                pongPayload.pongSenderAddress += (UInt16)(msg[8] << 8);
                pongPayload.pongSenderAddress += (UInt16)(msg[9]);

                //Convert byte array to ping receiver's address
                pongPayload.pongReceiverAddress += (UInt16)(msg[10] << 8);
                pongPayload.pongReceiverAddress += (UInt16)(msg[11]);

                return pongPayload;
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
        // Set up thread sync to shift between send_ping and receive modules
        // canPingProceed controls sending of pings. If set to true, pings are sent.
        // After sending a ping, canPingProceed is set to false. 
        // Receive checks if canPingProceed is set to false and waits until a pong is received.
        // Upon receiving a pong, canPingProceed is set to true. Send_Ping sends the next ping.
        private static readonly AutoResetEvent canPingProceed = new AutoResetEvent(true);
        private static readonly AutoResetEvent canPongProceed = new AutoResetEvent(false);    

        //const UInt16 MAX_NEIGHBORS = 12;
        NetOpStatus status;
        Timer pongNotReceivedTimer;
        EmoteLCD lcd;

        //UInt16[] neighborList;
        bool startSend = false;
        UInt16 myAddress;
        UInt16 dutyCyclePeriod = 20000;
        UInt16 countDownTimerDueValue = 10000;
        UInt16 countDownTimerDueTime;
        UInt32 totalPingCount = 200;
                
        static UInt32 sendMsgCounter = 0;
        static UInt32 totalSendMsgCounter = 0;
        static UInt32 totalRecvCounter = 0;
        
        PingPayload pingMsg = new PingPayload();
        PongPayload pongMsg = new PongPayload();

        OMAC myOMACObj;
        
        public void Initialize()
        {
            countDownTimerDueTime = (UInt16)(dutyCyclePeriod + countDownTimerDueValue);
            //Init LCD
            lcd = new EmoteLCD();
            lcd.Initialize();
            lcd.Write(LCD.CHAR_I, LCD.CHAR_n, LCD.CHAR_i, LCD.CHAR_t);

            try
            {
                Debug.Print("Initializing radio");
#if RF231
                var radioConfig = new RF231RadioConfiguration(RF231TxPower.Power_0Point0dBm, RF231Channel.Channel_13);
#elif SI4468
            var radioConfig = new SI4468RadioConfiguration(SI4468TxPower.Power_15Point5dBm, SI4468Channel.Channel_01);
#endif

                Debug.Print("Configuring OMAC...");
                //configure OMAC
                myOMACObj = new OMAC(radioConfig, 360);
                myOMACObj.OnReceive += Receive;
                myOMACObj.OnNeighborChange += NeighborChange;
            }
            catch (Exception e)
            {
                Debug.Print(e.ToString());
            }

            Debug.Print("OMAC init done");
            myAddress = myOMACObj.MACRadioObj.RadioAddress;
            Debug.Print("My address is: " + myAddress.ToString() + ". I send pings and receive pongs");
        }

        //Keeps track of change in neighborhood
        public void NeighborChange(IMAC macBase, DateTime time)
        {
            //Debug.Print("Count of neighbors " + countOfNeighbors.ToString());
        }

        //Starts a timer 
        public void Start()
        {
            Debug.Print("Starting timer...");
            TimerCallback timerCB = new TimerCallback(pongNotReceivedTimerCallback);
            pongNotReceivedTimer = new Timer(timerCB, null, countDownTimerDueTime, countDownTimerDueTime);
            Debug.Print("Timer initialization done");
            Debug.Print("Starting SendPing");
            SendPing();
        }

        //Calls ping at regular intervals
        void pongNotReceivedTimerCallback(Object obj)
        {
            Debug.Print("Timer countdown reached");
            //Send previous message
            sendMsgCounter--;
            SendPing();
        }

        public void SendPing()
        {
            //Before starting to send pings to neighbor, get list of neighbors
            bool startApp = false;
            
            while (!startApp)
            {
                UInt16[] neighborList = OMAC.NeighborListArray();
                DeviceStatus dsStatus = myOMACObj.NeighborList(neighborList);
                if (dsStatus == DeviceStatus.Success)
                { 
                    foreach (var neighbor in neighborList)
                    {
                        if (neighbor != 0)
                        {
                            startApp = true;
                            break;
                        }
                    }
                }
            }

            canPingProceed.Set();
            canPongProceed.Reset();
            
            //After getting list of neighbors, start sending them pings, until totalPingCount is reached
            //Send a ping only when a pong is received
            while (sendMsgCounter < totalPingCount)
            {
                try
                {
                    canPingProceed.WaitOne();

                    bool sendFlag = false;
                    UInt16[] neighborList = OMAC.NeighborListArray();
                    DeviceStatus dsStatus = myOMACObj.NeighborList(neighborList);
                    
                    //Set the timer to go off after countDownTimerDueTime before starting to send ping
                    Debug.Print("Resetting timer in SendPing");
                    pongNotReceivedTimer.Change(countDownTimerDueTime, countDownTimerDueTime);

                    Thread.Sleep(dutyCyclePeriod);

                    sendMsgCounter++; totalSendMsgCounter++;
                    Debug.Print("Total msgs sent: " + totalSendMsgCounter);

                    if (dsStatus == DeviceStatus.Success)
                    {
                        foreach (var neighbor in neighborList)
                        {
                            if (neighbor != 0)
                            {
                                startSend = true; sendFlag = true;
                                pingMsg.pingMsgId = sendMsgCounter;
                                pingMsg.pingSenderAddress = myAddress;
                                byte[] msg = pingMsg.ToBytes();
                                Debug.Print("Sending to neighbor " + neighbor + " ping msgID " + sendMsgCounter);

                                status = myOMACObj.Send(neighbor, PayloadType.MFM_Data, msg, 0, (ushort)msg.Length);
                                if (status != NetOpStatus.S_Success)
                                {
                                    Debug.Print("Send failed. Ping msgID " + sendMsgCounter.ToString());
                                }
                            }
                        }
                    }
                    else
                    {
                        Debug.Print("Could not get neighbor list");
                    }
                    if (sendFlag == false && startSend == true)
                    {
                        Debug.Print("Ping failed. All neighbors dropped out");
                        pongNotReceivedTimer.Change(Timeout.Infinite, Timeout.Infinite);
                        //Stop both ping and pong
                        //canPingProceed.Reset();
                        //canPongProceed.Reset();
                    }

                    canPingProceed.Reset();
                    canPongProceed.Set();

                    /*if (sendMsgCounter < 10)
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
                    }*/
                }
                catch (Exception ex)
                {
                    Debug.Print("SendPing: " + ex.ToString());
                }
            }
        }

        //Handles received messages 
        public void Receive(IMAC macBase, DateTime time, Packet receivedPacket)
        {
            canPongProceed.WaitOne();

            //Change timer countdown as soon a packet is received. This effectively tells ping that 
            //a pong has been received and that a ping need not be sent.
            /*Debug.Print("Resetting timer in Receive");
            pongNotReceivedTimer.Change(countDownTimerDueTime, countDownTimerDueTime);*/

            /*if (myOMACObj.GetPendingPacketCount_Receive() == 0)
            {
                Debug.Print("no packets");
                return;
            }

            Packet receivedPacket = myOMACObj.GetNextPacket();*/
            if (receivedPacket == null)
            {
                Debug.Print("null");
                return;
            }

            byte[] rcvPayload = receivedPacket.Payload;
            PongPayload pongPayload = pongMsg.FromBytesToPongPayload(rcvPayload);
            if (pongPayload != null)
            {
                if (pongPayload.pongReceiverAddress == myAddress)
                {
                    totalRecvCounter++;
                    Debug.Print("---------------------------");
                    Debug.Print("totalRecvCounter is " + totalRecvCounter);
                    Debug.Print("Received msgID " + pongPayload.pongMsgId);
                    Debug.Print("Received msgContent " + pongPayload.pongMsgContent.ToString());
                    Debug.Print("Received address " + pongPayload.pongSenderAddress.ToString());
                    Debug.Print("---------------------------");

                    bool result = pongPayload.pongMsgContent.Equals("PONG");
                    if (result)
                    {
                        if (totalRecvCounter == totalPingCount)
                        {
                            ShowStatistics();
                            //Disable ping and pong
                            canPongProceed.Reset();
                            canPingProceed.Reset();
                        }
                    }
                    else
                    {
                        Debug.Print("Invalid msg content. Sending ping again");
                        sendMsgCounter--;
                    }
                    //Enable ping and disable pong
                    canPongProceed.Reset();
                    canPingProceed.Set();
                }
                else
                {
                    Debug.Print("Ignoring pong from sender " + pongPayload.pongSenderAddress + " with msgId " + pongPayload.pongMsgId);
                    Debug.Print("Sending ping again");
                    sendMsgCounter--;
                    //Enable ping and disable pong
                    canPongProceed.Reset();
                    canPingProceed.Set();
                }
            }
            else
            {
                Debug.Print("Received a null msg. Sending ping again");
                sendMsgCounter--;
                //Enable ping and disable pong
                canPongProceed.Reset();
                canPingProceed.Set();
            }
        }

        //Show statistics
        void ShowStatistics()
        {
            Debug.Print("==============STATS================");
            Debug.Print("msg count sent " + sendMsgCounter);
            Debug.Print("total msgs sent " + totalSendMsgCounter);
            Debug.Print("total msgs received " + totalRecvCounter);
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
