/*
 * EmoteLoraHat.h
 *
 *  Created on: Aug 18, 2017
 *      Author: Bora
 */

#ifndef MICROFRAMEWORKPK_V4_3_DEVICECODE_TARGETS_NATIVE_STM32F10X_DEVICECODE_DRIVERS_RADIO_SX1276_EMOTELORAHAT_H_
#define MICROFRAMEWORKPK_V4_3_DEVICECODE_TARGETS_NATIVE_STM32F10X_DEVICECODE_DRIVERS_RADIO_SX1276_EMOTELORAHAT_H_

// Hardware stuff
#include <tinyhal.h>
#include <stm32f10x.h>
#include <spi\netmf_spi.h>


#define SX1276_CMD_ID_READ_RX_FIFO 0x77
#define SX1276_CMD_ID_WRITE_TX_FIFO 0x66


static int LORA_ASSERT(int x, const char *err) {
//	if (!x) {
//		si446x_debug_print(ERR100, "%s", err);
//		radio_error();
//		return 1;
//	}
//	else return 0;
	if(!x) return 1;
	else return 0;
}


//namespace SX1276_Semtech {

enum {
	NONE00=0,
	DEBUG01=1,
	DEBUG02=2,
	DEBUG03=3,
	ERR99=99,		// For things that are bad unless you know what you are doing.
	ERR100=100,
};
enum { CTS_TIMEOUT = 262143*2, CTS_VAL_GOOD=0xFF, CTS_WAIT=255 };
enum { VERB0=0, VERB1=1, ERR0=128, ERR1=256 };
enum { ROMC2A=6, ROMB1B=3 };

const unsigned si4468x_debug_level = ERR100; // CHANGE ME.


class LoraHardwareConfig{
public:
	enum LoraDaugterBoards{
		LORA_GPS_HAT,
	};

	struct SX1276_pin_setup_t{
//        PinName mosi;		//mosi_pin
//		PinName miso;  		//miso_pin
//		PinName sclk;		//sclk_pin
//		PinName nss;
//		PinName reset;
//        PinName dio0;
//		PinName dio1;
//		PinName dio2;
//		PinName dio3;
//		PinName dio4;
//		PinName dio5;


		GPIO_TypeDef 	*nirq_port0;
		uint16_t		nirq_pin0;
		GPIO_PIN		nirq_mf_pin0;
		GPIO_TypeDef 	*nirq_port1;
		uint16_t		nirq_pin1;
		GPIO_PIN		nirq_mf_pin1;
		GPIO_TypeDef 	*nirq_port2;
		uint16_t		nirq_pin2;
		GPIO_PIN		nirq_mf_pin2;
		GPIO_TypeDef 	*nirq_port3;
		uint16_t		nirq_pin3;
		GPIO_PIN		nirq_mf_pin3;
		GPIO_TypeDef 	*nirq_port4;
		uint16_t		nirq_pin4;
		GPIO_PIN		nirq_mf_pin4;
		GPIO_TypeDef 	*nirq_port5;
		uint16_t		nirq_pin5;
		GPIO_PIN		nirq_mf_pin5;


		GPIO_TypeDef 	*reset_port;
		uint16_t		reset_pin;
		GPIO_PIN		reset_mf_pin;

		SPI_TypeDef 	*spi_base;
		GPIO_TypeDef 	*spi_port;
		uint32_t		spi_rcc;
		GPIO_TypeDef 	*cs_port;
		uint16_t		cs_pin;
		uint16_t		sclk_pin;
		uint16_t		miso_pin;
		uint16_t		mosi_pin;

	};

	struct InterruptPins_t{
		UINT8 DIO0;
		UINT8 DIO1;
		UINT8 DIO2;
		UINT8 DIO3;
		UINT8 DIO4;
		UINT8 DIO5;
	};

protected:
	// Stores the configuration of the spi
	SPI_CONFIGURATION config;
	static SX1276_pin_setup_t SX1276_pin_setup;
	static unsigned ctsWentHigh;

	uint8_t radio_spi_go(uint8_t data);
	void radio_spi_sel_no_assert();

private:
	void initSPI2(); //Initialize a SPI structure
	void init_pins();
	void reset();
	void init_interrupts();
public:
	static InterruptPins_t SX1276_interupt_pins;



	LoraHardwareConfig();

	void Initialize();

	void spi_write_bytes(unsigned count, const uint8_t *buf);

	void spi_read_bytes(unsigned count, uint8_t *buf);

	unsigned int radio_comm_PollCTS();

	void radio_spi_sel_assert();

	uint8_t radio_comm_GetResp(uint8_t byteCount, uint8_t* pData);

};






class Emote_Lora_Hat : public LoraHardwareConfig {
private:


public:
	Emote_Lora_Hat();
	virtual ~Emote_Lora_Hat();
//
//	void write_tx_fifo(uint8_t numBytes, uint8_t* pTxData) {
//	  radio_comm_WriteData( SX1276_CMD_ID_WRITE_TX_FIFO, 0, numBytes, pTxData );
//	}
//
//	void read_rx_fifo(uint8_t numBytes, uint8_t* pRxData) {
//	  radio_comm_ReadData( SX1276_CMD_ID_READ_RX_FIFO, 0, numBytes, pRxData );
//	}

	bool SpiInitialize();

	void radio_comm_WriteData(uint8_t cmd, unsigned pollCts, uint8_t byteCount, uint8_t* pData);
	void radio_comm_ReadData(uint8_t cmd, unsigned pollCts, uint8_t byteCount, uint8_t* pData);



};

//} /* namespace LoraHat */

#endif /* MICROFRAMEWORKPK_V4_3_DEVICECODE_TARGETS_NATIVE_STM32F10X_DEVICECODE_DRIVERS_RADIO_SX1276_EMOTELORAHAT_H_ */
