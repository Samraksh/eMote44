#include "stm32h7xx_hal.h" 

/* Definition for USARTx clock resources */
#define USARTx                      USART3
#define USARTx_CLK_ENABLE()         __HAL_RCC_USART3_CLK_ENABLE()
#define USARTx_RX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOD_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOD_CLK_ENABLE()

#define USARTx_FORCE_RESET()        __HAL_RCC_USART3_FORCE_RESET()
#define USARTx_RELEASE_RESET()      __HAL_RCC_USART3_RELEASE_RESET()

/* Definition for USARTx Pins */
#define USARTx_TX_PIN               GPIO_PIN_8
#define USARTx_TX_GPIO_PORT         GPIOD
#define USARTx_TX_AF                GPIO_AF7_USART3
#define USARTx_RX_PIN               GPIO_PIN_9
#define USARTx_RX_GPIO_PORT         GPIOD
#define USARTx_RX_AF                GPIO_AF7_USART3
#define USARTx_CK_PIN               GPIO_PIN_7
#define USARTx_CK_GPIO_PORT         GPIOD
#define USARTx_CK_AF                GPIO_AF7_USART3

/*!
 * \brief GPIOs Macro
 */

#define RCC_GPIO_CLK_ENABLE( __GPIO_PORT__ )              \
do {                                                    \
    switch( __GPIO_PORT__)                                \
    {                                                     \
      case GPIOA_BASE: __HAL_RCC_GPIOA_CLK_ENABLE(); break;    \
      case GPIOB_BASE: __HAL_RCC_GPIOB_CLK_ENABLE(); break;    \
      case GPIOC_BASE: __HAL_RCC_GPIOC_CLK_ENABLE(); break;    \
	  case GPIOD_BASE: __HAL_RCC_GPIOD_CLK_ENABLE(); break;    \
	  case GPIOE_BASE: __HAL_RCC_GPIOE_CLK_ENABLE(); break;    \
	  case GPIOF_BASE: __HAL_RCC_GPIOF_CLK_ENABLE(); break;    \
	  case GPIOG_BASE: __HAL_RCC_GPIOG_CLK_ENABLE(); break;    \
      case GPIOH_BASE: default:  __HAL_RCC_GPIOH_CLK_ENABLE(); \
    }                                                    \
  } while(0)  

#define RCC_GPIO_CLK_DISABLE( __GPIO_PORT__ )              \
do {                                                    \
    switch( __GPIO_PORT__)                                \
    {                                                     \
      case GPIOA_BASE: __HAL_RCC_GPIOA_CLK_DISABLE(); break;    \
      case GPIOB_BASE: __HAL_RCC_GPIOB_CLK_DISABLE(); break;    \
	  case GPIOC_BASE: __HAL_RCC_GPIOC_CLK_DISABLE(); break;    \
      case GPIOD_BASE: __HAL_RCC_GPIOD_CLK_DISABLE(); break;    \
	  case GPIOE_BASE: __HAL_RCC_GPIOE_CLK_DISABLE(); break;    \
	  case GPIOF_BASE: __HAL_RCC_GPIOF_CLK_DISABLE(); break;    \
	  case GPIOG_BASE: __HAL_RCC_GPIOG_CLK_DISABLE(); break;    \
      case GPIOH_BASE: default:  __HAL_RCC_GPIOH_CLK_DISABLE(); \
    }                                                    \
  } while(0) 


/* Definition for LoRa I/O */
#define RADIO_RESET_PORT                          GPIOD
#define RADIO_RESET_PIN                           GPIO_PIN_15

#define RADIO_MOSI_PORT                           GPIOA
#define RADIO_MOSI_PIN                            GPIO_PIN_7

#define RADIO_MISO_PORT                           GPIOA
#define RADIO_MISO_PIN                            GPIO_PIN_6

#define RADIO_SCLK_PORT                           GPIOA
#define RADIO_SCLK_PIN                            GPIO_PIN_5

#define RADIO_NSS_PORT                            GPIOD
#define RADIO_NSS_PIN                             GPIO_PIN_14

//#define RADIO_BUSY_PORT                           GPIOB
//#define RADIO_BUSY_PIN                            GPIO_PIN_3

#define RADIO_DIO_0_PORT                          GPIOG
#define RADIO_DIO_0_PIN                           GPIO_PIN_14

#define RADIO_DIO_1_PORT                          GPIOE
#define RADIO_DIO_1_PIN                           GPIO_PIN_14

#define RADIO_DIO_2_PORT                          GPIOG
#define RADIO_DIO_2_PIN                           GPIO_PIN_12

#define RADIO_DIO_3_PORT                          GPIOF
#define RADIO_DIO_3_PIN                           GPIO_PIN_3

#define RADIO_ANT_SWITCH_POWER_PORT               GPIOA
#define RADIO_ANT_SWITCH_POWER_PIN                GPIO_PIN_9

//#define DEVICE_SEL_PORT                           GPIOA
//#define DEVICE_SEL_PIN                            GPIO_PIN_4

//#define RADIO_LEDTX_PORT                           GPIOC
//#define RADIO_LEDTX_PIN                            GPIO_PIN_1

//#define RADIO_LEDRX_PORT                           GPIOC
//#define RADIO_LEDRX_PIN                            GPIO_PIN_0
/* Definition for SPIx clock resources */

#define SPIx                        SPI1
#define SPIx_CLK_ENABLE()           __HAL_RCC_SPI1_CLK_ENABLE()
#define DMAx_CLK_ENABLE()           __HAL_RCC_DMA2_CLK_ENABLE()
#define SPIx_SCK_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPIx_MISO_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define SPIx_MOSI_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()

#define SPIx_FORCE_RESET()          __HAL_RCC_SPI1_FORCE_RESET()
#define SPIx_RELEASE_RESET()        __HAL_RCC_SPI1_RELEASE_RESET()

/* Definition for SPIx Pins */
#define SPIx_SCK_PIN                GPIO_PIN_5
#define SPIx_SCK_GPIO_PORT          GPIOA
#define SPIx_SCK_AF                 GPIO_AF5_SPI1
#define SPIx_MISO_PIN               GPIO_PIN_6
#define SPIx_MISO_GPIO_PORT         GPIOA
#define SPIx_MISO_AF                GPIO_AF5_SPI1
#define SPIx_MOSI_PIN               GPIO_PIN_7
#define SPIx_MOSI_GPIO_PORT         GPIOA
#define SPIx_MOSI_AF                GPIO_AF5_SPI1

/* Definition for SPIx's DMA */
#define SPIx_TX_DMA_STREAM               DMA2_Stream3
#define SPIx_RX_DMA_STREAM               DMA2_Stream2

#define SPIx_TX_DMA_REQUEST              DMA_REQUEST_SPI1_TX
#define SPIx_RX_DMA_REQUEST              DMA_REQUEST_SPI1_RX

/* Definition for SPIx's NVIC */
#define SPIx_DMA_TX_IRQn                 DMA2_Stream3_IRQn
#define SPIx_DMA_RX_IRQn                 DMA2_Stream2_IRQn

#define SPIx_DMA_TX_IRQHandler           DMA2_Stream3_IRQHandler
#define SPIx_DMA_RX_IRQHandler           DMA2_Stream2_IRQHandler

/* Definition for SPIx's NVIC */
#define SPIx_IRQn                        SPI1_IRQn
#define SPIx_IRQHandler                  SPI1_IRQHandler

#define BUFFERSIZE                       (COUNTOF(aTxBuffer2) - 1)
/* Size of Trasmission buffer */
#define TXBUFFERSIZE                (COUNTOF(aTxBuffer) - 1)
/* Size of Reception buffer */
#define RXBUFFERSIZE                TXBUFFERSIZE
  
/* Exported macro ------------------------------------------------------------*/
#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))
/* Exported functions ------------------------------------------------------- */