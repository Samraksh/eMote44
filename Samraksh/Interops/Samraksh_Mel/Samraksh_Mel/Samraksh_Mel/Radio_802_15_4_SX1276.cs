using System;
using Microsoft.SPOT;

namespace Samraksh_Mel.Radio
{
    /// <summary>SX1276 radio object</summary>
    public class Radio_802_15_4_SX1276 : Radio_802_15_4_Base
    {
        private static bool _sx1276RadioInstanceSet;

        /// <summary>Constructor</summary>
        public Radio_802_15_4_SX1276()
        {
        }

        /// <summary>Constructor</summary>
        /// <param name="user">Radio user</param>
        public Radio_802_15_4_SX1276(RadioUser user)
        {
            if (!_sx1276RadioInstanceSet)
            {
                _sx1276RadioInstanceSet = true;
                if (user == RadioUser.CSMA)
                {
                    CurrUser = RadioUser.CSMA;
                }
                else if (user == RadioUser.OMAC)
                {
                    CurrUser = RadioUser.OMAC;
                }
                else if (user == RadioUser.CSharp)
                {
                    CurrUser = RadioUser.CSharp;
                }
            }
            else
                Debug.Print("Long range radio (SX1276) already initialized");
        }

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        public DeviceStatus Radio_802_15_4_SX1276_UnInitialize()
        {
            _sx1276RadioInstanceSet = false;
            CurrUser = RadioUser.Idle;
            return DeviceStatus.Success;
        }

    }
}
