using Microsoft.SPOT;

// ReSharper disable once CheckNamespace
namespace Samraksh.eMote.Net.Radio
{
    /// <summary>Long range radio</summary>
    // ReSharper disable once InconsistentNaming
    public class Radio_802_15_4_RF231_LR : Radio_802_15_4_Base
    {
        private static bool _lrRadioInstanceSet;
        
        /// <summary>Constructor</summary>
        public Radio_802_15_4_RF231_LR()
        {
        }

        /// <summary>Constructor</summary>
        /// <param name="user"></param>
        public Radio_802_15_4_RF231_LR(RadioUser user)
        {
            if (!_lrRadioInstanceSet)
            {
                _lrRadioInstanceSet = true;
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
                Debug.Print("Long range radio (RF231) already initialized");
        }

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        public DeviceStatus Radio_802_15_4_RF231_LR_UnInitialize()
        {
            _lrRadioInstanceSet = false;
            CurrUser = RadioUser.Idle;
            return DeviceStatus.Success;
            //Radio is uninitialized when MAC is uninitialized
            //return UnInitialize((byte)RadioName.RF231LR);
        }

    }
}
