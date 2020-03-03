#ifndef __SX1276_INTEROP_DEFINITIONS_H__
#define __SX1276_INTEROP_DEFINITIONS_H__

    /// Power levels supported by the SX1276 radio
    enum SX1276TxPower
    {
        /// <summary>14 dB</summary>
        Power_14dBm = 31,
        /// <summary>13 dB</summary>
        Power_13dBm = 30,
        /// <summary>12 dB</summary>
        Power_12dBm = 29,
        /// <summary>11 dB</summary>
        Power_11dBm = 28,
        /// <summary>10 dB</summary>
        Power_10dBm = 27,
        /// <summary>9 dB</summary>
        Power_9dBm = 26,
        /// <summary>8 dB</summary>
        Power_8dBm = 25,
        /// <summary>7 dB</summary>
        Power_7dBm = 24,
        /// <summary>6 dB</summary>
        Power_6dBm = 23,
        /// <summary>5 dB</summary>
        Power_5dBm = 22,
        /// <summary>4 dB</summary>
        Power_4dBm = 21,
        /// <summary>3 dB</summary>
        Power_3dBm = 20,
        /// <summary>2 dB</summary>
        Power_2dBm = 19,
        /// <summary>1 dB</summary>
        Power_1dBm = 18,
        /// <summary>0 dB</summary>
        Power_0dBm = 17,
        /// <summary>-1 dB</summary>
        Power_Minus_1dBm = 16,
        /// <summary>-2 dB</summary>
        Power_Minus_2dBm = 15,
        /// <summary>-3 dB</summary>
        Power_Minus_3dBm = 14,
        /// <summary>-4 dB</summary>
        Power_Minus_4dBm = 13
    };

    /// Bandwidth supported by SX1276 radio
    enum SX1276Bandwidth
    {
        /// <summary>Bandwidth 125kHz</summary>
        Bandwidth_125kHz = 0,
        /// <summary>Bandwidth 250kHz</summary>
        Bandwidth_250kHz = 1,
        /// <summary>Bandwidth 500kHz</summary>
        Bandwidth_500kHz = 2,
    };

    /// Coding rate supported by SX1276 radio
    enum SX1276CodingRate
    {
        /// <summary>Coding Rate 4/5</summary>
        CodingRate_4_5 = 0,
        /// <summary>Coding Rate 4/6</summary>
        CodingRate_4_6 = 1,
        /// <summary>Coding Rate 4/7</summary>
        CodingRate_4_7 = 2,
        /// <summary>Coding Rate 4/8</summary>
        CodingRate_4_8 = 3
    };

    /// Spreading factors supported by SX1276 radio
    enum SX1276SpreadingFactor
    {
        /// <summary>Spreading Factor 7</summary>
        SpreadingFactor_7 = 0,
        /// <summary>Spreading Factor 8</summary>
        SpreadingFactor_8 = 1,
        /// <summary>Spreading Factor 9</summary>
        SpreadingFactor_9 = 2,
        /// <summary>Spreading Factor 10</summary>
        SpreadingFactor_10 = 3,
        /// <summary>Spreading Factor 11</summary>
        SpreadingFactor_11 = 4,
        /// <summary>Spreading Factor 12</summary>
        SpreadingFactor_12 = 5
    };

    /// Channels supported by SX1276 radio
    enum SX1276Channel
    {
        /// <summary>902 MHz</summary>
        Channel_902 = 0,
        /// <summary>903 MHz</summary>
        Channel_903 = 1,
        /// <summary>904 MHz</summary>
        Channel_904 = 2,
        /// <summary>905 MHz</summary>
        Channel_905 = 3,
        /// <summary>906 MHz</summary>
        Channel_906 = 4,
        /// <summary>907 MHz</summary>
        Channel_907 = 5,
        /// <summary>908 MHz</summary>
        Channel_908 = 6,
        /// <summary>909 MHz</summary>
        Channel_909 = 7,
        /// <summary>910 MHz</summary>
        Channel_910 = 8,
        /// <summary>911 MHz</summary>
        Channel_911 = 9,
        /// <summary>912 MHz</summary>
        Channel_912 = 10,
        /// <summary>913 MHz</summary>
        Channel_913 = 11,
        /// <summary>914 MHz</summary>
        Channel_914 = 12,
        /// <summary>915 MHz</summary>
        Channel_915 = 13,
        /// <summary>916 MHz</summary>
        Channel_916 = 14,
        /// <summary>917 MHz</summary>
        Channel_917 = 15,
        /// <summary>918 MHz</summary>
        Channel_918 = 16,
        /// <summary>919 MHz</summary>
        Channel_919 = 17,
        /// <summary>920 MHz</summary>
        Channel_920 = 18,
        /// <summary>921 MHz</summary>
        Channel_921 = 19,
        /// <summary>922 MHz</summary>
        Channel_922 = 20,
        /// <summary>923 MHz</summary>
        Channel_923 = 21,
        /// <summary>924 MHz</summary>
        Channel_924 = 22,
        /// <summary>925 MHz</summary>
        Channel_925 = 23,
        /// <summary>926 MHz</summary>
        Channel_926 = 24,
        /// <summary>927 MHz</summary>
        Channel_927 = 25,
        /// <summary>928 MHz</summary>
        Channel_928 = 26,
    };


#endif
