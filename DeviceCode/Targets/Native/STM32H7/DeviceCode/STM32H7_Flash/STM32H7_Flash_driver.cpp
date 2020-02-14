////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//  Implementation for STM32H7: Copyright (c) Oberon microsystems, Inc.
//
//  *** STM32H7 Flash Driver ***
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "STM32H7_Flash.h"
#include <stm32h7xx_hal.h>
#include "..\stm32h7xx.h"
#include <stm32h7xx_hal_flash_ex.h>

#ifdef __cplusplus
extern "C" {
#endif
	 extern HAL_StatusTypeDef HAL_FLASHEx_Erase(FLASH_EraseInitTypeDef *pEraseInit, uint32_t *SectorError);
	 extern HAL_StatusTypeDef HAL_FLASH_Program(uint32_t TypeProgram, uint32_t FlashAddress, uint32_t DataAddress);
#ifdef __cplusplus
}
#endif


#ifndef FLASH
#define FLASH               ((FLASH_TypeDef *) FLASH_R_BASE)
#endif

typedef UINT32 CHIP_WORD;

#define FLASH_CR_PSIZE_BITS FLASH_CR_PSIZE_1 // 32 bit programming
    
#if SUPPLY_VOLTAGE_MV < 2700
    #error 32 bit Flash programming not allowed for voltages below 2.7V
#endif
#if SYSTEM_CYCLE_CLOCK_HZ < ONE_MHZ
    #error Flash programming not allowed for HCLK below 1MHz
#endif


static const UINT32 STM32H7_FLASH_KEY1 = 0x45670123;
static const UINT32 STM32H7_FLASH_KEY2 = 0xCDEF89AB;

/**
  * @brief  Gets the page of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The page of a given address
  */
static inline uint32_t GetPage(uint32_t Addr)
{
  if IS_FLASH_PROGRAM_ADDRESS_BANK1(Addr)
  {
    /* Bank 1 */
    return (Addr - FLASH_BANK1_BASE) / FLASH_SECTOR_SIZE;
  }
  else
  {
    /* Bank 2 */
    return (Addr - FLASH_BANK2_BASE) / FLASH_SECTOR_SIZE;
  }
}

/**
  * @brief  Gets the bank of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The bank of a given address
  */
static inline uint32_t GetBank(uint32_t Addr)
{
  uint32_t bank = 0;

  if (READ_BIT(SYSCFG->UR0, SYSCFG_UR0_BKS) == 0)
  {
  	/* No Bank swap */
    if IS_FLASH_PROGRAM_ADDRESS_BANK1(Addr)
    {
      bank = FLASH_BANK_1;
    }
    else
    {
      bank = FLASH_BANK_2;
    }
  }
  else
  {
  	/* Bank swap */
    if IS_FLASH_PROGRAM_ADDRESS_BANK1(Addr)
    {
      bank = FLASH_BANK_2;
    }
    else
    {
      bank = FLASH_BANK_1;
    }
  }

  return bank;
}


//--//

/////////////////////////////////////////////////////////
// Description:
//    Initializes a given block device for use
// 
// Input:
//
// Returns:
//   true if successful; false if not
//
// Remarks:
//    No other functions in this interface may be called
//    until after Init returns.
//
BOOL __section("SectionForFlashOperations")STM32H7_Flash_Driver::ChipInitialize( void* context )
{
    NATIVE_PROFILE_HAL_DRIVERS_FLASH();
    return TRUE;
}

/////////////////////////////////////////////////////////
// Description:
//    Initializes a given block device for use
// 
// Returns:
//   true if successful; false if not
//
// Remarks:
//   De initializes the device when no longer needed
//
BOOL __section("SectionForFlashOperations")STM32H7_Flash_Driver::ChipUnInitialize( void* context )
{
    NATIVE_PROFILE_HAL_DRIVERS_FLASH();
    return TRUE;
}

/////////////////////////////////////////////////////////
// Description:
//    Gets the information describing the device
//
const BlockDeviceInfo* __section("SectionForFlashOperations")STM32H7_Flash_Driver::GetDeviceInfo( void* context )
{
    MEMORY_MAPPED_NOR_BLOCK_CONFIG* config = (MEMORY_MAPPED_NOR_BLOCK_CONFIG*)context;

    return config->BlockConfig.BlockDeviceInformation;
}

/////////////////////////////////////////////////////////
// Description:
//    Reads data from a set of sectors
//
// Input:
//    StartSector - Starting Sector for the read
//    NumSectors  - Number of sectors to read
//    pSectorBuff - pointer to buffer to read the data into.
//                  Must be large enough to hold all of the data
//                  being read.
//
// Returns:
//   true if successful; false if not
//
// Remarks:
//   This function reads the number of sectors specified from the device.
//
BOOL  __section("SectionForFlashOperations")STM32H7_Flash_Driver::Read( void* context, ByteAddress StartSector, UINT32 NumBytes, BYTE * pSectorBuff)
{
    // XIP device does not need to read into a buffer
    NATIVE_PROFILE_HAL_DRIVERS_FLASH();

    if (pSectorBuff == NULL) return FALSE;

    CHIP_WORD* ChipAddress = (CHIP_WORD *)StartSector;
    CHIP_WORD* EndAddress  = (CHIP_WORD *)(StartSector + NumBytes);
    CHIP_WORD *pBuf        = (CHIP_WORD *)pSectorBuff;

    while(ChipAddress < EndAddress)
    {
        *pBuf++ = *ChipAddress++;
    }

    return TRUE;
}


/////////////////////////////////////////////////////////
// Description:
//    Writes data to a set of sectors
//
// Input:
//    StartSector - Starting Sector for the write
//    NumSectors  - Number of sectors to write
//    pSectorBuff - pointer to data to write.
//                  Must be large enough to hold complete sectors
//                  for the number of sectors being written.
//
// Returns:
//   true if successful; false if not
//
// Remarks:
//   This function reads the number of sectors specified from the device.
//
BOOL __section("SectionForFlashOperations")STM32H7_Flash_Driver::Write(void* context, ByteAddress Address, UINT32 NumBytes, BYTE * pSectorBuff, BOOL ReadModifyWrite)
{
    NATIVE_PROFILE_PAL_FLASH();

	UINT32 *ChipAddress = (UINT32 *)Address;
	UINT32 *EndAddress = (UINT32 *)(Address + NumBytes);
	UINT32 *pBuf        = (UINT32 *)pSectorBuff;
    // Read-modify-write is used for FAT filesystems only
    if (ReadModifyWrite) return FALSE;

    HAL_FLASH_Unlock();
	
	while (ChipAddress < EndAddress)
	{
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, (UINT32)ChipAddress, (UINT32)pBuf) == HAL_OK)
		{
		  ChipAddress+=8; /* increment for the next Flash word*/
		  pBuf+=8;
		  //debug_printf( "end 0x%08x, 0x%08x\r\n",(UINT32)ChipAddress, (UINT32)pBuf);
		  
		}
		else 
		{
			debug_printf( "wrong 0x%08x, 0x%08x\r\n",(UINT32)ChipAddress, (UINT32)pBuf);
			break;			
		}
	}
	
	HAL_FLASH_Lock();
	  
	/*if (FLASH->CR1 & FLASH_CR_LOCK) { // unlock
        FLASH->KEYR1 = STM32H7_FLASH_KEY1;
        FLASH->KEYR1 = STM32H7_FLASH_KEY2;
    }
    
	CHIP_WORD* ChipAddress = (CHIP_WORD *)Address;
    CHIP_WORD* EndAddress  = (CHIP_WORD *)(Address + NumBytes);
    CHIP_WORD *pBuf        = (CHIP_WORD *)pSectorBuff;

    // enable programming
    FLASH->CR1 = FLASH_CR_PG | FLASH_CR_PSIZE_BITS;

	debug_printf( "end 0x%08x, 0x%08x\r\n",(UINT32)EndAddress, (UINT32)pBuf);
    
	while(ChipAddress < EndAddress) {
        if (*ChipAddress != *pBuf) {
            // write data
            // *ChipAddress = *pBuf;
			*ChipAddress = 0x01020304;
			__ISB();
            __DSB();
            // wait for completion
            while (FLASH->SR1 & FLASH_SR_BSY);
            // check
            if (*ChipAddress != *pBuf) {
                debug_printf( "Flash_WriteToSector failure @ 0x%08x, wrote 0x%08x, read 0x%08x\r\n", (UINT32)ChipAddress, *pBuf, *ChipAddress );
                return FALSE;
            }
        }
        ChipAddress++;
        pBuf++;
    }

    // reset & lock the controller
    FLASH->CR1 = FLASH_CR_LOCK;
   */
   
    return TRUE;
}

BOOL __section("SectionForFlashOperations")STM32H7_Flash_Driver::Memset(void* context, ByteAddress Address, UINT8 Data, UINT32 NumBytes)
{
    NATIVE_PROFILE_PAL_FLASH();

    // used for FAT filesystems only
    return FALSE;
}

BOOL __section("SectionForFlashOperations")STM32H7_Flash_Driver::GetSectorMetadata(void* context, ByteAddress SectorStart, SectorMetadata* pSectorMetadata)
{
    // no metadata
    return FALSE;
}

BOOL __section("SectionForFlashOperations")STM32H7_Flash_Driver::SetSectorMetadata(void* context, ByteAddress SectorStart, SectorMetadata* pSectorMetadata)
{
    // no metadata
    return FALSE;
}


/////////////////////////////////////////////////////////
// Description:
//    Check a block is erased or not.
// 
// Input:
//    BlockStartAddress - Logical Sector Address
//
// Returns:
//   true if it is erassed, otherwise false
//
// Remarks:
//    Check  the block containing the sector address specified.
//
BOOL __section("SectionForFlashOperations")STM32H7_Flash_Driver::IsBlockErased( void* context, ByteAddress BlockStart, UINT32 BlockLength )
{
    NATIVE_PROFILE_HAL_DRIVERS_FLASH();

    CHIP_WORD* ChipAddress = (CHIP_WORD *) BlockStart;
    CHIP_WORD* EndAddress  = (CHIP_WORD *)(BlockStart + BlockLength);
    
    while(ChipAddress < EndAddress)
    {
        if(*ChipAddress != (CHIP_WORD)-1)
        {
            return FALSE;
        }
        ChipAddress++;
    }

    return TRUE;
}


/////////////////////////////////////////////////////////
// Description:
//    Erases a block
// 
// Input:
//    Address - Logical Sector Address
//
// Returns:
//   true if successful; false if not
//
// Remarks:
//    Erases the block containing the sector address specified.
//
BOOL __section("SectionForFlashOperations")STM32H7_Flash_Driver::EraseBlock( void* context, ByteAddress address )
{
  	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t EraseError = 0;

	// Get the 1st page to erase
  	const uint32_t StartSector = GetPage(address);
  	// Get the number of pages to erase from 1st page
	uint32_t bytesPerBlock;
	if (address > FLASH_BASE_ADDRESS3){
		bytesPerBlock = FLASH_BYTES_PER_BLOCK3;
	} else if (address > FLASH_BASE_ADDRESS2){
		bytesPerBlock = FLASH_BYTES_PER_BLOCK2;
	} else {
		// assume block 1
		bytesPerBlock = FLASH_BYTES_PER_BLOCK1;
	}
  	const uint32_t NbOfSectors = GetPage(address + FLASH_BYTES_PER_BLOCK3) - StartSector + 1;
  	// Get the bank 
  	const uint32_t BankNumber = GetBank(address);

	// Clear pending flags (if any)
  	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_BANK1);
  	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_BANK2);

	// Unlock the Flash to enable the flash control register access
	HAL_FLASH_Unlock();

	// Clear all error flags
  if(BankNumber == FLASH_BANK_1)
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS_BANK1);
  else
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS_BANK2);

  // Fill EraseInit structure
  EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
  EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
  EraseInitStruct.Banks         = BankNumber;
  EraseInitStruct.Sector        = StartSector;
  EraseInitStruct.NbSectors     = NbOfSectors;

  /* Note: If an erase operation in Flash memory also concerns data in the data or instruction cache,
     you have to make sure that these data are rewritten before they are accessed during code
     execution. If this cannot be done safely, it is recommended to flush the caches by setting the
     DCRST and ICRST bits in the FLASH_CR register. */
  if (HAL_FLASHEx_Erase(&EraseInitStruct, &EraseError) != HAL_OK)
  {
    return HAL_FLASH_GetError();
  }

  HAL_FLASH_Lock();

  return TRUE;
}



/////////////////////////////////////////////////////////
// Description:
//   Changes the power state of the device
// 
// Input:
//    State   - true= power on; false = power off
//
// Remarks:
//   This function allows systems to conserve power by 
//   shutting down the hardware when the system is 
//   going into low power states.
//
void __section("SectionForFlashOperations")STM32H7_Flash_Driver::SetPowerState( void* context, UINT32 State )
{
}


//--// ---------------------------------------------------

#pragma arm section code = "SectionForFlashOperations"

UINT32 __section("SectionForFlashOperations")STM32H7_Flash_Driver::MaxSectorWrite_uSec( void* context )
{
    NATIVE_PROFILE_PAL_FLASH();

    MEMORY_MAPPED_NOR_BLOCK_CONFIG* config = (MEMORY_MAPPED_NOR_BLOCK_CONFIG*)context;

    return config->BlockConfig.BlockDeviceInformation->MaxSectorWrite_uSec;
}


UINT32 __section("SectionForFlashOperations")STM32H7_Flash_Driver::MaxBlockErase_uSec( void* context )
{
    NATIVE_PROFILE_PAL_FLASH();

    MEMORY_MAPPED_NOR_BLOCK_CONFIG* config = (MEMORY_MAPPED_NOR_BLOCK_CONFIG*)context;

    return config->BlockConfig.BlockDeviceInformation->MaxBlockErase_uSec;
}


#if defined(ADS_LINKER_BUG__NOT_ALL_UNUSED_VARIABLES_ARE_REMOVED)
#pragma arm section rodata = "g_STM32H7_Flash_DeviceTable"
#endif

struct IBlockStorageDevice g_STM32H7_Flash_DeviceTable =
{
    &STM32H7_Flash_Driver::ChipInitialize,
    &STM32H7_Flash_Driver::ChipUnInitialize,
    &STM32H7_Flash_Driver::GetDeviceInfo,
    &STM32H7_Flash_Driver::Read,
    &STM32H7_Flash_Driver::Write,
    &STM32H7_Flash_Driver::Memset,
    &STM32H7_Flash_Driver::GetSectorMetadata,
    &STM32H7_Flash_Driver::SetSectorMetadata,
    &STM32H7_Flash_Driver::IsBlockErased,
    &STM32H7_Flash_Driver::EraseBlock,
    &STM32H7_Flash_Driver::SetPowerState,
    &STM32H7_Flash_Driver::MaxSectorWrite_uSec,
    &STM32H7_Flash_Driver::MaxBlockErase_uSec,
};

#if defined(ADS_LINKER_BUG__NOT_ALL_UNUSED_VARIABLES_ARE_REMOVED)
#pragma arm section rodata
#endif
