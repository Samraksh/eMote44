/* Copyright: The Samaraksh Company (All rights reserved)
 * Author: Mukundan Sridharan, Ananth Muralidharan
 * Date: Feb 2013, Nov 2013
 * Function: Provides access to Samraksh NVM DataStore filesystem
 */

using System;
using System.Runtime.CompilerServices;


// ReSharper disable once CheckNamespace
namespace Samraksh.eMote.NonVolatileMemory
{
    /// <summary>
    /// Block device type
    /// </summary>
    public enum StorageType
    {
        NOR = 1,
        SD
    };

    /// <summary>
    /// DataStore error values 
    /// </summary>
    public enum DataStoreError
    {
        DataStoreErrorNone,
        DataStoreErrorInvalidParam,
        DataStoreErrorOutOfMemory,
        DataStoreErrorInvalidDataId,
        DataStoreErrorInvalidGivenAddr,
        DataStoreErrorOutOfBoundAccess,
        DataStoreErrorOutOfFlashMemory,
        DataStoreErrorDataIdAlreadyExists,
        DataStoreErrorUnexpectedError
    };

    /// <summary>
    /// Data types allowed in DataStore
    /// </summary>
    public enum ReferenceDataType
    {
        BYTE,
        UINT16,
        UINT32
    };

    //AnanthAtSamraksh - to be kept in sync with the one in Samraksh_SPOT_Samraksh_SPOT_NonVolatileMemory_DataStore.cpp
    /// <summary>
    /// DataStore return types
    /// </summary>
    public enum DataStoreReturnStatus
    {
        Success = 0,
        Failure = -1,
        InvalidArgument = -2,
        InvalidReference = -3,
        DataStoreNotInitialized = -4,
        DataAllocationOutOfMemory = -5
        //AlreadyExists,
        //InvalidPointer
    };

    /// <summary>
    /// Different exception classes based on return value from native methods
    /// </summary>
    public class DataStoreInvalidArgumentException : Exception
    {
        public DataStoreInvalidArgumentException()
        {
        }

        public DataStoreInvalidArgumentException(string message)
            : base(message)
        {
        }

        public DataStoreInvalidArgumentException(string message, Exception inner)
            : base(message, inner)
        {
        }
    }

    /// <summary>
    /// 
    /// </summary>
    public class DataStoreInvalidReferenceException : Exception
    {
        public DataStoreInvalidReferenceException()
        {
        }

        public DataStoreInvalidReferenceException(string message)
            : base(message)
        {
        }

        public DataStoreInvalidReferenceException(string message, Exception inner)
            : base(message, inner)
        {
        }
    }

    /// <summary>
    /// 
    /// </summary>
    public class DataStoreNotInitializedException : Exception
    {
        public DataStoreNotInitializedException()
        {
        }

        public DataStoreNotInitializedException(string message)
            : base(message)
        {
        }

        public DataStoreNotInitializedException(string message, Exception inner)
            : base(message, inner)
        {
        }
    }

    /// <summary>
    /// 
    /// </summary>
    public class DataStoreOutOfMemoryException : Exception
    {
        public DataStoreOutOfMemoryException()
        {
        }

        public DataStoreOutOfMemoryException(string message)
            : base(message)
        {
        }

        public DataStoreOutOfMemoryException(string message, Exception inner)
            : base(message, inner)
        {
        }
    }

    /// <summary>
    /// 
    /// </summary>
    public class DataStoreException : Exception
    {
        public DataStoreException()
        {
        }

        public DataStoreException(string message)
            : base(message)
        {
        }

        public DataStoreException(string message, Exception inner)
            : base(message, inner)
        {
        }
    }
    
}
