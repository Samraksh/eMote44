using System;
using System.Runtime.CompilerServices;


namespace Samraksh.eMote.NonVolatileMemory
{
    /// <summary>
    /// DataStore class to store data. 
    /// Can be any implemented block storage device.
    /// </summary>
    public class DataStore
    {

        /// <summary>
        /// The block storage device type
        /// </summary>
        private StorageType storageType;

        /// <summary>
        /// The only instance of DataStore.
        /// </summary>
        private static DataStore DSInstance;
        /// <summary>
        /// Counter for multiton pattern 
        /// </summary>
        //private static int counter = 0;
        /// <summary>
        /// For synchronization between multiple threads
        /// </summary>
        private static object syncObject = new Object();
        /// <summary>
        /// HashTable for storing multiple instances of DataStore initialized to different block storage devices
        /// </summary>
        private static System.Collections.Hashtable dataStoreInstances = new System.Collections.Hashtable();

        /// <summary>
        /// Private constructor so that only one instance of DataStore exists.
        /// </summary>
        private DataStore() { }

        /// <summary>
        /// Multiton pattern. This allows DataStore to be initialized with different block storage devices such as NOR, NAND etc.
        /// http://en.wikipedia.org/wiki/Multiton_pattern
        /// </summary>
        public static DataStore Instance(StorageType storageTypeKey, bool eraseDataStore = false)
        {
            if (!dataStoreInstances.Contains(storageTypeKey))
            {
                lock (syncObject)
                {
                    DSInstance = new DataStore();
                    int retVal = DSInstance.InitDataStore(storageTypeKey, eraseDataStore);
                    if (retVal != 0)
                        return null;
                    dataStoreInstances.Add(storageTypeKey, DSInstance);
                }
            }
            return (DataStore)(dataStoreInstances[storageTypeKey]);
        }

        /// <summary>
        /// Initializes DataStore with specified storage type.
        /// </summary>
        /// <param name="storageType">Storage type</param>
        /// <param name="eraseDataStore">Should datastore be erased?</param>
        private int InitDataStore(StorageType storageType, bool eraseDataStore)
        {
            this.storageType = storageType;
            return CreateDataStore(eraseDataStore);
        }

        //////////////////////////public properties and methods/////////////////////

        /// <summary>
        /// Maximum allocation size possible in bytes
        /// </summary>
        public int MaxAllocationSize
        {
            get
            {
                return GetMaxAllocationSize();
            }
        }

        /// <summary>
        /// Size of DataStore in bytes
        /// </summary>
        public int Size
        {
            get
            {
                return GetTotalSpace();
            }
        }

        /// <summary>
        /// Number of bytes of space already occupied by data
        /// </summary>
        public int UsedBytes
        {
            get
            {
                return GetUsedBytes();
            }
        }

        /// <summary>
        /// Number of bytes of space available to store data
        /// </summary>
        public int FreeBytes
        {
            get
            {
                return GetFreeBytes();
            }
        }


        /// <summary>
        /// Fills an array with data references from specified offset.
        /// </summary>
        /// <param name="dataRefArray">Data reference array</param>
        /// <param name="dataIdOffset">Offset from first data reference in DataStore</param>
        /// <returns>Success / failure status</returns>
        /// <remarks>
        ///     Typically an offset variable. passed as the second argument, is initialized to zero 
        ///     and incremented by the size of the data reference array after each invocation of the method. 
        ///     This will iteratively return all data references. 
        ///     If, after invocation, the array is null then no more data references.
        /// </remarks>
        /// <exception caption="DataStore Exception" cref="DataStoreException">Method invocation has an invalid argument</exception>
        public DataStoreReturnStatus ReadAllDataReferences(DataReference[] dataRefArray, int dataIdOffset)
        {
            UInt32 dataId;
            if (dataRefArray.Length == 0)
                throw new DataStoreException("dataRefArray cannot be of zero length");

            if (dataIdOffset < 0)
                throw new DataStoreException("dataIdOffset should not be negative");

            /* User checks if array contents are null to break out of program. 
                * Clear the contents of array so that null values are returned if there is no data allocation in DataStore. */
            Array.Clear(dataRefArray, 0, dataRefArray.Length);
            int[] dataIdArray = new int[dataRefArray.Length];

            int retVal = GetReadAllDataIds(dataIdArray, dataIdArray.Length, dataIdOffset);

            if (retVal != 0)
                return DataStoreReturnStatus.Failure;
            else
            {
                for (UInt16 arrayIndex = 0; arrayIndex < dataRefArray.Length; ++arrayIndex)
                {
                    dataId = (UInt32)dataIdArray[arrayIndex];
                    if (dataId != 0)
                    {
                        try
                        {
                            dataRefArray[arrayIndex] = new DataReference(this, (int)dataId);
                        }
                        catch (Exception)
                        {
                            return DataStoreReturnStatus.Failure;
                        }
                    }
                }
            }
            return DataStoreReturnStatus.Success;
        }

        /// <summary>
        /// Deletes all data references from DataStore. Does not actually erase the data, but marks them as inactive.
        /// </summary>
        /// <returns>Success / failure status</returns>
        public DataStoreReturnStatus DeleteAllData()
        {
            int retVal = DeleteAll();

            if (retVal == 0)
                return DataStoreReturnStatus.Success;
            else if (retVal == -3)
                return DataStoreReturnStatus.InvalidReference;
            else
                return DataStoreReturnStatus.Failure;
        }

        /// <summary>
        /// Performs garbage collection on DataStore. This is done internally by DataStore when there is not enough space to add new data.
        /// GC rearranges data internally so that all active data are grouped together. Then the inactive data are erased one block at a time.
        /// </summary>
        /// <returns>Success / failure status</returns>
        private DataStoreReturnStatus GC()
        {
            if (DataStoreGC() == true)
                return DataStoreReturnStatus.Success;
            else
                return DataStoreReturnStatus.Failure;
        }

        /// <summary>
        /// Deletes all data references from DataStore and erases them.
        /// </summary>
        /// <returns>Success / failure status</returns>
        public DataStoreReturnStatus EraseAllData()
        {
            // Remove contents from AddressTable also before erasing DataStore
            int retVal = DeleteAll();

            if (retVal == 0)
            {
                //DeleteAll was successful
                retVal = EraseAllBlocks();

                if (retVal == 0)
                    return DataStoreReturnStatus.Success;
                else if (retVal == -3)
                    return DataStoreReturnStatus.InvalidReference;
                else
                    return DataStoreReturnStatus.Failure;
            }
            else if (retVal == -3)
                return DataStoreReturnStatus.InvalidReference;
            else
                return DataStoreReturnStatus.Failure;
        }


        /// <summary>
        /// Write data array to DataStore starting from specified address
        /// </summary>
        /// <param name="address">Address to write to</param>
        /// <param name="data">Data to be written to DataStore</param>
        /// <param name="offset">Offset from start of DataStore allocationto write.</param>
        /// <param name="numBytes">Count of bytes to be written</param>
        /// <param name="dataType">Data type to be written</param>
        /// <returns>True if success</returns>
        [Obsolete("Deprecated. Use WriteRaw instead.")]
        public int Write(UInt32 address, byte[] data, UInt32 offset, UInt32 numBytes, byte dataType)
        {
            return WriteRaw(address, data, offset, numBytes, dataType, (int)storageType);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="address"></param>
        /// <param name="data"></param>
        /// <param name="offset"></param>
        /// <param name="numBytes"></param>
        /// <param name="dataType"></param>
        /// <returns></returns>
        [Obsolete("Deprecated. Use WriteRaw instead.")]
        public int Write(UInt32 address, byte[] data, UInt32 offset, UInt32 numBytes, UInt16 dataType)
        {
            return WriteRaw(address, data, offset, numBytes, dataType, (int)storageType);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="address"></param>
        /// <param name="data"></param>
        /// <param name="offset"></param>
        /// <param name="numBytes"></param>
        /// <param name="dataType"></param>
        /// <returns></returns>
        [Obsolete("Deprecated. Use WriteRaw instead.")]
        public int Write(UInt32 address, byte[] data, UInt32 offset, UInt32 numBytes, UInt32 dataType)
        {
            return WriteRaw(address, data, offset, numBytes, dataType, (int)storageType);
        }


        /// <summary>
        /// Write data array to DataStore starting from specified address
        /// </summary>
        /// <param name="address">Address to write to</param>
        /// <param name="data">Data to be written to DataStore</param>
        /// <param name="offset">Offset from start of DataStore allocationto write.</param>
        /// <param name="numBytes">Count of bytes to be written</param>
        /// <param name="dataType">Data type to be written</param>
        /// <returns>True if success</returns>
        public int WriteRaw(UInt32 address, byte[] data, UInt32 offset, UInt32 numBytes, byte dataType)
        {
            return WriteRaw(address, data, offset, numBytes, dataType, (int)storageType);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="address"></param>
        /// <param name="data"></param>
        /// <param name="offset"></param>
        /// <param name="numBytes"></param>
        /// <param name="dataType"></param>
        /// <returns></returns>
        public int WriteRaw(UInt32 address, byte[] data, UInt32 offset, UInt32 numBytes, UInt16 dataType)
        {
            return WriteRaw(address, data, offset, numBytes, dataType, (int)storageType);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="address"></param>
        /// <param name="data"></param>
        /// <param name="offset"></param>
        /// <param name="numBytes"></param>
        /// <param name="dataType"></param>
        /// <returns></returns>
        public int WriteRaw(UInt32 address, byte[] data, UInt32 offset, UInt32 numBytes, UInt32 dataType)
        {
            return WriteRaw(address, data, offset, numBytes, dataType, (int)storageType);
        }

        /// <summary>
        /// Read data array from DataStore starting from specified address.
        /// </summary>
        /// <param name="address">Address to read from</param>
        /// <param name="data">Byte array to be filled up with data from DataStore</param>
        /// <param name="offset">Offset from start of DataStore allocationto read.</param>
        /// <param name="numBytes">Number of bytes to be read</param>
        /// <param name="dataType">Data type of the data to be read</param>
        /// <returns>True if success</returns>
        [Obsolete("Deprecated. Use ReadRaw instead.")]
        public int Read(UInt32 address, byte[] data, UInt32 offset, UInt32 numBytes, byte dataType)
        {
            return ReadRaw(address, data, offset, numBytes, dataType, (int)storageType);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="address"></param>
        /// <param name="data"></param>
        /// <param name="offset"></param>
        /// <param name="numBytes"></param>
        /// <param name="dataType"></param>
        /// <returns></returns>
        [Obsolete("Deprecated. Use ReadRaw instead.")]
        public int Read(UInt32 address, byte[] data, UInt32 offset, UInt32 numBytes, UInt16 dataType)
        {
            return ReadRaw(address, data, offset, numBytes, dataType, (int)storageType);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="address"></param>
        /// <param name="data"></param>
        /// <param name="offset"></param>
        /// <param name="numBytes"></param>
        /// <param name="dataType"></param>
        /// <returns></returns>
        [Obsolete("Deprecated. Use ReadRaw instead.")]
        public int Read(UInt32 address, byte[] data, UInt32 offset, UInt32 numBytes, UInt32 dataType)
        {
            return ReadRaw(address, data, offset, numBytes, dataType, (int)storageType);
        }


        /// <summary>
        /// Read data array from DataStore starting from specified address.
        /// </summary>
        /// <param name="address">Address to read from</param>
        /// <param name="data">Byte array to be filled up with data from DataStore</param>
        /// <param name="offset">Offset from start of DataStore allocationto read.</param>
        /// <param name="numBytes">Number of bytes to be read</param>
        /// <param name="dataType">Data type of the data to be read</param>
        /// <returns>True if success</returns>
        public int ReadRaw(UInt32 address, byte[] data, UInt32 offset, UInt32 numBytes, byte dataType)
        {
            return ReadRaw(address, data, offset, numBytes, dataType, (int)storageType);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="address"></param>
        /// <param name="data"></param>
        /// <param name="offset"></param>
        /// <param name="numBytes"></param>
        /// <param name="dataType"></param>
        /// <returns></returns>
        public int ReadRaw(UInt32 address, byte[] data, UInt32 offset, UInt32 numBytes, UInt16 dataType)
        {
            return ReadRaw(address, data, offset, numBytes, dataType, (int)storageType);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="address"></param>
        /// <param name="data"></param>
        /// <param name="offset"></param>
        /// <param name="numBytes"></param>
        /// <param name="dataType"></param>
        /// <returns></returns>
        public int ReadRaw(UInt32 address, byte[] data, UInt32 offset, UInt32 numBytes, UInt32 dataType)
        {
            return ReadRaw(address, data, offset, numBytes, dataType, (int)storageType);
        }

        /// <summary>
        /// Destroys the data in DataStore
        /// </summary>
        ~DataStore()
        {
            //DisposeNativeMemoryPointer(dataId);
        }

        ///////////////////////////////////Internal methods/////////////////////////

        /// <summary>
        /// Initializes DataStore. Performs a scan of the device and builds up the list of active data inside DataStore.
        /// </summary>
        /// <returns>True if success</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private int CreateDataStore(bool eraseDataStore);

        /// <summary>
        /// Method that returns maximum allocation size possible in DataStore
        /// </summary>
        /// <returns>Maximum allocation size possible in DataStore</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private int GetMaxAllocationSize();

        /// <summary>
        /// Get total available space 
        /// </summary>
        /// <returns>Amount of total space</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private int GetTotalSpace();

        /// <summary>
        /// Get amount of used space 
        /// </summary>
        /// <returns>Amount of used space</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private int GetUsedBytes();

        /// <summary>
        /// Get amount of free space
        /// </summary>
        /// <returns>Amount of free space</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private int GetFreeBytes();

        /// <summary>
        /// Returns valid data currently stored in DataStore
        /// </summary>
        /// <param name="dataIdArray">Data ID array</param>
        /// <param name="arrayLength">Size of array</param>
        /// <param name="dataIdOffset">Offset</param>
        /// <returns>Array with valid data as unsigned integer</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private int GetReadAllDataIds(int[] dataIdArray, int arrayLength, int dataIdOffset);

        /// <summary>
        /// Get the count of valid data currently stored in DataStore
        /// </summary>
        /// <returns>Count of valid data as unsigned integer</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private int GetCountOfDataIds();

        /// <summary>
        /// Delete all references to data in DataStore
        /// </summary>
        /// <returns>True if success</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private int DeleteAll();

        /// <summary>
        /// Erases all data in DataStore
        /// </summary>
        /// <returns>True if success</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private int EraseAllBlocks();

        /// <summary>
        /// Garbage collects inactive data in DataStore
        /// </summary>
        /// <returns>True if success</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private bool DataStoreGC();

        /// <summary>
        /// Gets read/write status of DataStore
        /// </summary>
        /// <returns>True if status indicates success</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private bool GetReadWriteStatus();

        /// <summary>
        /// Gets error status of DataStore
        /// </summary>
        /// <returns>DataStore error status</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private int GetLastDatastoreStatus();

        //Returns the size of the NVM, if 0 operation failed
        //[MethodImplAttribute(MethodImplOptions.InternalCall)]
        //extern private UInt32 ConstructNativeMemoryPointer(UInt32 dataId, UInt32 bytes);

        /// <summary>
        /// Write data present in buffer starting from address and amount equal to numBytes. 
        /// </summary>
        /// <param name="address">Address from which to start writing data</param>
        /// <param name="data">Buffer holding data to be written</param>
        /// <param name="offset">Offset from start of DataStore allocationto read.</param>
        /// <param name="numBytes">Amount of data to be written</param>
        /// <param name="dataType">Data type of the data to be written</param>
        /// <param name="storageType">Block storage type</param>
        /// <returns>True if success</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private int WriteRaw(UInt32 address, byte[] data, UInt32 offset, UInt32 numBytes, byte dataType, int storageType);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private int WriteRaw(UInt32 address, byte[] data, UInt32 offset, UInt32 numBytes, UInt16 dataType, int storageType);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private int WriteRaw(UInt32 address, byte[] data, UInt32 offset, UInt32 numBytes, UInt32 dataType, int storageType);


        /// <summary>
        /// Read from data into the buffer starting from address.
        /// </summary>
        /// <param name="address">Address of data to be read from</param>
        /// <param name="buffer">Buffer into which the data will be read</param>
        /// <param name="offset">Offset from start of DataStore allocationto read.</param>
        /// <param name="numBytes">Number of bytes to be read</param>
        /// <param name="dataType">Data type of the data to be read</param>
        /// <param name="storageType">Block storage type</param>
        /// <returns>Number of bytes read; -1 if operation fails</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private int ReadRaw(UInt32 address, byte[] buffer, UInt32 offset, UInt32 numBytes, byte dataType, int storageType);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private int ReadRaw(UInt32 address, byte[] buffer, UInt32 offset, UInt32 numBytes, UInt16 dataType, int storageType);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private int ReadRaw(UInt32 address, byte[] buffer, UInt32 offset, UInt32 numBytes, UInt32 dataType, int storageType);


        // native call that destroys data created on the flash
        //[MethodImplAttribute(MethodImplOptions.InternalCall)]
        //extern private bool DisposeNativeMemoryPointer(UInt32 dataId);

    }

}
