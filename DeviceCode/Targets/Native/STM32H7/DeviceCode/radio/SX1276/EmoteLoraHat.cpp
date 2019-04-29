/*
 * EmoteLoraHat.cpp
 *
 *  Created on: Aug 18, 2017
 *      Author: Bora
 */

#include "EmoteLoraHat.h"
//namespace SX1276_Semtech {

#ifndef SX1276_NO_DEBUG_PRINT
static void SX1276_debug_print(int priority, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    if(priority >= si4468x_debug_level) {
		hal_vprintf(fmt, args);
	}

    va_end(args);
}
#else
// Compiler is smart enough to toss out debug strings even in debug mode
static void SX1276_debug_print(int priority, const char *fmt, ...) { return; }
#endif


// Temporary hack.
static unsigned get_APB1_clock() {
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);
	return RCC_Clocks.PCLK1_Frequency;
}

static unsigned get_APB2_clock() {
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);
	return RCC_Clocks.PCLK2_Frequency;
}

Emote_Lora_Hat::Emote_Lora_Hat() {
	// TODO Auto-generated constructor stub
	Initialize();
}

bool Emote_Lora_Hat::SpiInitialize() {
//	initSPI2();
	// Calling mf spi initialize function
	//if(TRUE != CPU_SPI_Initialize())
	//	return FALSE;

//	config.DeviceCS               = 10; //TODO - fix me..
//	config.CS_Active              = false;
//	config.CS_Setup_uSecs         = 0;
//	config.CS_Hold_uSecs          = 0;
//	config.MSK_IDLE               = false;
//	config.MSK_SampleEdge         = false;
//	config.Clock_RateKHz          = 16; // THIS IS IGNORED.
//	if(this->GetRadioName() == RF231RADIO)
//	{
//		config.SPI_mod                = RF231_SPI_BUS;
//	}
//	else if(this->GetRadioName() == RF231RADIOLR)
//	{
//		config.SPI_mod 				  = RF231_LR_SPI_BUS;
//	}
//
//	config.MD_16bits = FALSE;
//
//
//	// Enable the SPI depending on the radio who is the user
//	CPU_SPI_Enable(config);

	CPU_GPIO_SetPinState( (GPIO_PIN)25 , FALSE);
	CPU_GPIO_SetPinState( (GPIO_PIN)25 , TRUE);

	return TRUE;

}

Emote_Lora_Hat::~Emote_Lora_Hat() {
	// TODO Auto-generated destructor stub
}

void Emote_Lora_Hat::radio_comm_WriteData(uint8_t cmd, unsigned pollCts,
		uint8_t byteCount, uint8_t* pData) {
    if(pollCts)
    {
        while(!ctsWentHigh)
        {
            radio_comm_PollCTS();
        }
    }
	radio_spi_sel_assert(); //cs pin chip select
	radio_spi_go(cmd);
	spi_write_bytes(byteCount, pData);
	radio_spi_sel_no_assert();

    //radio_hal_ClearNsel();
    //radio_hal_SpiWriteByte(cmd);
    //radio_hal_SpiWriteData(byteCount, pData);
    //radio_hal_SetNsel();

    ctsWentHigh = 0;
}

void Emote_Lora_Hat::radio_comm_ReadData(uint8_t cmd, unsigned pollCts,
		uint8_t byteCount, uint8_t* pData) {
	if (pollCts) { // Not sure of case where you wouldn't want to do this
		radio_comm_PollCTS();
	}
	radio_spi_sel_assert();
	radio_spi_go(cmd);
	spi_read_bytes(byteCount, pData);
	radio_spi_sel_no_assert();
	ctsWentHigh = 0;
}



LoraHardwareConfig::LoraHardwareConfig() {



	SX1276_pin_setup.nirq_port0			= GPIOA;
	SX1276_pin_setup.nirq_pin0			= GPIO_Pin_1;
	SX1276_pin_setup.nirq_mf_pin0		= (GPIO_PIN) 1;

	SX1276_pin_setup.nirq_port1			= GPIOA;
	SX1276_pin_setup.nirq_pin1			= GPIO_Pin_2;
	SX1276_pin_setup.nirq_mf_pin1		= (GPIO_PIN) 2;

	SX1276_pin_setup.nirq_port2			= GPIOA;
	SX1276_pin_setup.nirq_pin2			= GPIO_Pin_3;
	SX1276_pin_setup.nirq_mf_pin2		= (GPIO_PIN) 3;

	SX1276_pin_setup.nirq_port3			= GPIOA;
	SX1276_pin_setup.nirq_pin3			= GPIO_Pin_8;
	SX1276_pin_setup.nirq_mf_pin3		= (GPIO_PIN) 8;


	SX1276_pin_setup.nirq_port4			= GPIOA;
	SX1276_pin_setup.nirq_pin4			= GPIO_Pin_0; //GPIO_Pin_22; // TODO:
	SX1276_pin_setup.nirq_mf_pin4		= (GPIO_PIN) 22;

	SX1276_pin_setup.nirq_port5			= GPIOA;
	SX1276_pin_setup.nirq_pin5			= GPIO_Pin_0; //GPIO_Pin_23;
	SX1276_pin_setup.nirq_mf_pin5		= (GPIO_PIN) 23;

	SX1276_pin_setup.reset_port			= GPIOA; // ?
	SX1276_pin_setup.reset_pin			= GPIO_Pin_0; // GPIO_Pin_24;
	SX1276_pin_setup.reset_mf_pin		= (GPIO_PIN) 24;

	SX1276_pin_setup.spi_base 			= SPI2;
	SX1276_pin_setup.spi_port 			= GPIOB;
	SX1276_pin_setup.cs_port			= GPIOA;
	SX1276_pin_setup.cs_pin				= GPIO_Pin_4;
	SX1276_pin_setup.sclk_pin			= GPIO_Pin_13;
	SX1276_pin_setup.miso_pin			= GPIO_Pin_14;
	SX1276_pin_setup.mosi_pin			= GPIO_Pin_15;

	SX1276_pin_setup.spi_rcc			= RCC_APB1Periph_SPI2;

	Initialize();
}

void LoraHardwareConfig::reset(){
	CPU_GPIO_SetPinState(SX1276_pin_setup.reset_mf_pin, TRUE);
	CPU_GPIO_SetPinState(SX1276_pin_setup.reset_mf_pin, FALSE);
}


void LoraHardwareConfig::init_pins(){
	GPIO_InitTypeDef GPIO_InitStructure;

	SX1276_pin_setup_t *config = &SX1276_pin_setup;

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin =  config->reset_pin;
	GPIO_Init(config->reset_port, &GPIO_InitStructure);



	// NIRQ
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = config->nirq_pin0;
	GPIO_Init(config->nirq_port0, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = config->nirq_pin1;
	GPIO_Init(config->nirq_port1, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = config->nirq_pin2;
	GPIO_Init(config->nirq_port2, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = config->nirq_pin3;
	GPIO_Init(config->nirq_port3, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = config->nirq_pin4;
	GPIO_Init(config->nirq_port4, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = config->nirq_pin5;
	GPIO_Init(config->nirq_port5, &GPIO_InitStructure);

	// PA4 SPI chip select
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = config->cs_pin;
	GPIO_WriteBit(config->cs_port, config->cs_pin, Bit_SET); // Set
	GPIO_Init(config->cs_port, &GPIO_InitStructure);
}

void LoraHardwareConfig::Initialize(){

	initSPI2();
	init_pins();
	reset();
}

void LoraHardwareConfig::initSPI2() {
	GPIO_InitTypeDef GPIO_InitStructure;
	unsigned int baud;
	unsigned SpiBusClock;
	SPI_InitTypeDef SPI_InitStruct;

	RCC_APB1PeriphClockCmd(SX1276_pin_setup.spi_rcc,	ENABLE);
	SPI_I2S_DeInit(SX1276_pin_setup.spi_base);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin =  SX1276_pin_setup.sclk_pin | SX1276_pin_setup.mosi_pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(SX1276_pin_setup.spi_port, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = SX1276_pin_setup.miso_pin;
	GPIO_Init(SX1276_pin_setup.spi_port, &GPIO_InitStructure);

	SPI_StructInit(&SPI_InitStruct);
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_CRCPolynomial = 7;

	// FIX ME, TEMP
	if (SX1276_pin_setup.spi_base != SPI1)
		SpiBusClock = get_APB1_clock();
	else
		SpiBusClock = get_APB2_clock();

	switch(SPI_InitStruct.SPI_BaudRatePrescaler) {
		case SPI_BaudRatePrescaler_2:  baud = SpiBusClock / 2  / 1000; break;
		case SPI_BaudRatePrescaler_4:  baud = SpiBusClock / 4  / 1000; break;
		case SPI_BaudRatePrescaler_8:  baud = SpiBusClock / 8  / 1000; break;
		case SPI_BaudRatePrescaler_16: baud = SpiBusClock / 16 / 1000; break;
		default: baud = 0;
	}

	SPI_Init(SX1276_pin_setup.spi_base, &SPI_InitStruct);
	SPI_Cmd(SX1276_pin_setup.spi_base, ENABLE);

	if (SX1276_pin_setup.spi_base == SPI2) {
		SX1276_debug_print(DEBUG02,"SPI2 up CPOL: %d CPHA: %d Baud: %d kHz (%d kHz bus)\r\n",
			SPI_InitStruct.SPI_CPOL, SPI_InitStruct.SPI_CPHA, baud, SpiBusClock/1000);
	}
	else {
		SX1276_debug_print(DEBUG02,"SPI??? up CPOL: %d CPHA: %d Baud: %d kHz (%d kHz bus)\r\n",
			SPI_InitStruct.SPI_CPOL, SPI_InitStruct.SPI_CPHA, baud, SpiBusClock/1000);
	}
}

void LoraHardwareConfig::spi_write_bytes(unsigned count,
		const uint8_t* buf)  {
	for(unsigned i=0; i<count; i++) {
		radio_spi_go(buf[i]);
	}
}

uint8_t LoraHardwareConfig::radio_spi_go(uint8_t data)  {
	while( SPI_I2S_GetFlagStatus(SX1276_pin_setup.spi_base, SPI_I2S_FLAG_TXE) == RESET ) ; // spin
	SPI_I2S_SendData(SX1276_pin_setup.spi_base, data);
	while( SPI_I2S_GetFlagStatus(SX1276_pin_setup.spi_base, SPI_I2S_FLAG_RXNE) == RESET ) ; // spin
	return SPI_I2S_ReceiveData(SX1276_pin_setup.spi_base);
}

void LoraHardwareConfig::radio_spi_sel_no_assert() {
	GPIO_WriteBit(SX1276_pin_setup.cs_port, SX1276_pin_setup.cs_pin, Bit_SET); // chip select
}

void LoraHardwareConfig::spi_read_bytes(unsigned count, uint8_t* buf)  {
	for(unsigned i=0; i<count; i++) {
		buf[i] = radio_spi_go(0);
	}
}

unsigned int LoraHardwareConfig::radio_comm_PollCTS(){
	unsigned timeout=0;

#ifdef SI446X_AGGRESSIVE_CTS
	if (ctsWentHigh) return 1;
#else
	ctsWentHigh = 0;
#endif

	while(ctsWentHigh == 0 && timeout < CTS_TIMEOUT) {
		radio_comm_GetResp(0, NULL);
		timeout++;
	}
	if (timeout == CTS_TIMEOUT) {
		ctsWentHigh = 0;
		LORA_ASSERT(0, "Fatal: CTS Timeout waiting for response\r\n");
		return 0;
	}

	return 1;
}

void LoraHardwareConfig::radio_spi_sel_assert() {
	GPIO_WriteBit(SX1276_pin_setup.cs_port, SX1276_pin_setup.cs_pin, Bit_RESET); // chip select
	__NOP();
}

uint8_t LoraHardwareConfig::radio_comm_GetResp(uint8_t byteCount,
		uint8_t* pData) {
	unsigned ctsVal;
	unsigned timeout=0;

	radio_spi_sel_assert();
	radio_spi_go(0x44); //read CMD buffer
	ctsVal = radio_spi_go(0);

	while(ctsVal != 0xFF && timeout++ <= CTS_TIMEOUT) {
		radio_spi_sel_no_assert();
		//for(unsigned i=0; i<CTS_WAIT; i++) ; // spin
		// Looking for at least 150ns, or likely even half that would be enough.
		__NOP(); __NOP(); __NOP(); __NOP(); __NOP();
		__NOP(); __NOP(); __NOP(); __NOP(); __NOP();
		__NOP(); __NOP(); __NOP(); __NOP(); __NOP();
		radio_spi_sel_assert();
		radio_spi_go(0x44); //read CMD buffer
		ctsVal = radio_spi_go(0);
	}

	if (ctsVal != 0xFF) {
		radio_spi_sel_no_assert();
		LORA_ASSERT(0, "Fatal: CTS Timeout waiting for response\r\n");
		return 0;
	}
	else {
		ctsWentHigh = 1;
	}

	if (byteCount) {
		spi_read_bytes(byteCount, pData);
	}
	radio_spi_sel_no_assert();

	return ctsVal;
}

//} /* namespace LoraHat */
