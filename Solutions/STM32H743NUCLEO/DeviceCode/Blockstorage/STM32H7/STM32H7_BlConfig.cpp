////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//  Implementation for the MCBSTM32F400 board (STM32F4): Copyright (c) Oberon microsystems, Inc.
//
//  *** STM32H743NUCLEO Block Storage Configuration ***
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <tinyhal.h>



const BlockRange g_STM32H7_BlockRange1[] =
{
    { BlockRange::BLOCKTYPE_BOOTSTRAP ,   0, 0 },  // 08000000 bootloader  128k
    { BlockRange::BLOCKTYPE_CODE      ,   1, 14 }, // 08020000 CLR         1792k (inc 4k config)
	{ BlockRange::BLOCKTYPE_CONFIG    ,   13,13 }, // 081DF000 Config      4k. This must exist.
    { BlockRange::BLOCKTYPE_DEPLOYMENT,   15,15 }, // 081E0000 deployment  128k
};

const BlockRegionInfo  g_STM32H7_BlkRegion[STM32H7__NUM_REGIONS] = 
{
    {
        FLASH_BASE_ADDRESS1,    // ByteAddress   Start;           // Starting Sector address
        FLASH_BLOCK_COUNT1,     // UINT32        NumBlocks;       // total number of blocks in this region
        FLASH_BYTES_PER_BLOCK1, // UINT32        BytesPerBlock;   // Total number of bytes per block
        ARRAYSIZE_CONST_EXPR(g_STM32H7_BlockRange1),
        g_STM32H7_BlockRange1,
    }
};

const BlockDeviceInfo g_STM32H7_DeviceInfo=
{
    {  
        STM32H7__IS_REMOVABLE,             // BOOL Removable;
        STM32H7__SUPPORTS_XIP,             // BOOL SupportsXIP;
        STM32H7__WRITE_PROTECTED,          // BOOL WriteProtected;
        STM32H7__SUPP_COPY_BACK            // BOOL SupportsCopyBack
    },
    FLASH_SECTOR_WRITE_TYPICAL_TIME_USEC,  // UINT32 MaxSectorWrite_uSec;
    FLASH_BLOCK_ERASE_ACTUAL_TIME_USEC,    // UINT32 MaxBlockErase_uSec;
    FLASH_BYTES_PER_SECTOR,                // UINT32 BytesPerSector;     

    FLASH_MEMORY_Size,                     // UINT32 Size;

    STM32H7__NUM_REGIONS,                  // UINT32 NumRegions;
    g_STM32H7_BlkRegion,                   // const BlockRegionInfo* pRegions;
};

struct MEMORY_MAPPED_NOR_BLOCK_CONFIG g_STM32H7_BS_Config =
{
    { // BLOCK_CONFIG
        {
            STM32H7__WP_GPIO_PIN,          // GPIO_PIN             Pin;
            STM32H7__WP_ACTIVE,            // BOOL                 ActiveState;
        },

        &g_STM32H7_DeviceInfo,             // BlockDeviceinfo
    },

    { // CPU_MEMORY_CONFIG
        STM32H7__CHIP_SELECT,              // UINT8  CPU_MEMORY_CONFIG::ChipSelect;
        TRUE,                              // UINT8  CPU_MEMORY_CONFIG::ReadOnly;
        STM32H7__WAIT_STATES,              // UINT32 CPU_MEMORY_CONFIG::WaitStates;
        STM32H7__RELEASE_COUNTS,           // UINT32 CPU_MEMORY_CONFIG::ReleaseCounts;
        STM32H7__BIT_WIDTH,                // UINT32 CPU_MEMORY_CONFIG::BitWidth;
        STM32H7__BASE_ADDRESS,             // UINT32 CPU_MEMORY_CONFIG::BaseAddress;
        STM32H7__SIZE_IN_BYTES,            // UINT32 CPU_MEMORY_CONFIG::SizeInBytes;
        0,                                 // UINT8  CPU_MEMORY_CONFIG::XREADYEnable 
        0,                                 // UINT8  CPU_MEMORY_CONFIG::ByteSignalsForRead 
        0,                                 // UINT8  CPU_MEMORY_CONFIG::ExternalBufferEnable
    },

    0,                                     // UINT32 ChipProtection;
    FLASH_MANUFACTURER_CODE,               // UINT32 ManufacturerCode;
    FLASH_DEVICE_CODE,                     // UINT32 DeviceCode;
};

struct BlockStorageDevice g_STM32H7_BS;

