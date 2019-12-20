using System;
using System.Runtime.CompilerServices;
using Microsoft.SPOT;
using Microsoft.SPOT.Hardware;

namespace Samraksh.eMote.ProgramManager
{

    /// <summary>
    /// provides lastest state of update transmission
    /// </summary>
    /// <param name="UpdateID"></param>
    /// <param name="state"></param>
    public delegate void UpdaterProgressCallback(UInt32 updateID, UpdateState state);


    public sealed class ProgramManager
    {

        #region MULTI-THREAD_SINGLETON
        private static /*volatile*/ ProgramManager instance;
        private static object syncRoot = new Object();

        private ProgramManager()
        {
            bool ret = UpdaterInternal.Configure("UpdaterProgressCallback", 0);
            _updaterInternal = UpdaterInternal.Instance;
        }

        public static ProgramManager Instance
        {
            get
            {
                if (instance == null)
                {
                    lock (syncRoot)
                    {
                        if (instance == null)
                            instance = new ProgramManager();
                    }
                }

                return instance;
            }
        }
        #endregion

        #region UPDATER

        private static UpdaterInternal _updaterInternal;

        static UpdaterProgressCallback _updaterProgressCallback;

        static NativeEventHandler _updaterProgressHandler;

        static private void InternalUpdaterProgressCallback(uint data1, uint data2, DateTime time)
        {
            //TODO: fetch UpdateState info for UpdateID
            //TODO: fill more state variables. keep track of some state across callbacks or query MFUpdate directly.
            UpdateState state = new UpdateState();
            UInt32 id = data1;
            state.updateID = id;
            state.destAddr = (ushort)(data2 >> 16);
            state.lastReceivedMsg = (UpdateMessage)((data2 & 0xF000) >> 12);
            UInt16 data = (ushort)(data2 & 0xFFF);

            if(state.lastReceivedMsg == UpdateMessage.ADDPACKET_ACK) {
                // TODO: get update header. or always send this as a progress metric.
                state.missingPacketCount = data;
            }

            state.latestActionDate = time;

            //TODO: show user how to grab more info about the update in C-Sharp.

            if (_updaterProgressCallback != null) {
                _updaterProgressCallback( id, state);  //TODO: process a list of callbacks for individual updateID, destAddr
            }
        }

        public void SetUpdaterProgressCallback(UInt32 updateID, UInt16 destAddr, UpdaterProgressCallback callback)
        {
            //TODO: create storage for obj second parameter of the callback.
            //TODO: consider creating a unique string so we can have more than one callback in the queue at a time for a given update.
            //TODO: create storage for (updateID,destAddr) tuples.
            SetUpdaterProgressCallback(callback);
        }

        public void SetUpdaterProgressCallback(UpdaterProgressCallback callback) {
            _updaterInternal = UpdaterInternal.Instance;
            _updaterProgressCallback = callback;  //TODO: use a list instead.
            if (_updaterProgressHandler == null) {
                _updaterProgressHandler = new NativeEventHandler(InternalUpdaterProgressCallback);
            }
            _updaterInternal.OnInterrupt += _updaterProgressHandler;
        }


        /// <summary>
        /// begins (re-)transmission of the first stored update
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern bool UpdaterStart();

        /// <summary>
        /// begins (re-)transmission of the specified update
        /// </summary>
        /// <param name="id">id of desired update</param>
        /// <returns>false if update not found</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern bool UpdaterStart(UInt32 id);

        /// <summary>
        /// set node id of where the update should be transmitted   
        /// </summary>
        /// <param name="addr">mote id destination address</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern void UpdaterSetDestination(UInt16 addr);

        #endregion
    }
}
