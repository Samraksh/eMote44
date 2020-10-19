#include <tinyhal.h>
#include <stm32h7xx_hal.h>
#include "..\stm32h7xx.h"

#ifndef STARTUP_DELAY_MS
#define START_UP_DELAY() ((void)0)
#else
#define START_UP_DELAY() if (STARTUP_DELAY_MS > 0) HAL_Delay(STARTUP_DELAY_MS)
#endif

// If we are a BASE, always use 480 MHz (max). If a FENCE, allow for throttling
#ifdef MKII_BASE_CONFIG
#define MAX_CLOCK_ONLY
#endif

#define BREAKPOINT(x) __asm__("BKPT")
//#define TINY_CLR_VECTOR_TABLE_OFFSET 0x00040000
#define VECT_TAB_OFFSET 0x00000070
//#if defined(TARGETLOCATION_RAM)
//extern UINT32 Load$$ER_RAM$$Base;
//#elif defined(TARGETLOCATION_FLASH)
//	extern const uint32_t _vectors_start_;
//	#define ROM_START ((uint32_t *)&_vectors_start_)
extern UINT32 Load$$ER_FLASH$$Base;
//#else
//    !ERROR
//#endif

// Note: Not an ISR
static void Error_Handler(void)
{
#ifdef DEBUG
	while(1)
	{
		__asm__("BKPT");
	}
#endif
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
// 480 MHz (Maximum clock, REV 'V' SILICON ONLY
static int is_fast_clock = -1;

void MaxSystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  if (is_fast_clock > 0 ) return;
  is_fast_clock = 1;

#ifndef MAX_CLOCK_ONLY
  CPU_USB_Uninitialize(0);

  HAL_RCC_DeInit();
#endif

  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 117;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 20;
  RCC_OscInitStruct.PLL.PLLR = 128;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 1536;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV16;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV16;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV16;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV16;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_UART5|RCC_PERIPHCLK_SPI3
                              |RCC_PERIPHCLK_SPI1|RCC_PERIPHCLK_USB
							  |RCC_PERIPHCLK_LPTIM2|RCC_PERIPHCLK_QSPI
                              |RCC_PERIPHCLK_LPTIM1|RCC_PERIPHCLK_FMC
                              |RCC_PERIPHCLK_CKPER;
  PeriphClkInitStruct.FmcClockSelection = RCC_FMCCLKSOURCE_PLL;
  PeriphClkInitStruct.QspiClockSelection = RCC_QSPICLKSOURCE_PLL;
  PeriphClkInitStruct.CkperClockSelection = RCC_CLKPSOURCE_HSE;
  PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_CLKP;
  PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
  PeriphClkInitStruct.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_LSE;
  PeriphClkInitStruct.Lptim2ClockSelection = RCC_LPTIM2CLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_PWREx_EnableUSBVoltageDetector();
}

// 60 MHz
#ifdef MAX_CLOCK_ONLY
void MinSystemClock_Config(void) { return; }
#else
void MinSystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  if (is_fast_clock == 0) return;
  is_fast_clock = 0;

  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  HAL_RCC_DeInit();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 29;
  RCC_OscInitStruct.PLL.PLLP = 4;
  RCC_OscInitStruct.PLL.PLLQ = 5;
  RCC_OscInitStruct.PLL.PLLR = 128;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOMEDIUM;
  RCC_OscInitStruct.PLL.PLLFRACN = 2432;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV16;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;  // NOTE DIFFERENT
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV16;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV16;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_UART5|RCC_PERIPHCLK_SPI3
                              |RCC_PERIPHCLK_SPI1|RCC_PERIPHCLK_USB
							  |RCC_PERIPHCLK_LPTIM2|RCC_PERIPHCLK_QSPI
                              |RCC_PERIPHCLK_LPTIM1|RCC_PERIPHCLK_FMC
                              |RCC_PERIPHCLK_CKPER;
  PeriphClkInitStruct.FmcClockSelection = RCC_FMCCLKSOURCE_PLL;
  PeriphClkInitStruct.QspiClockSelection = RCC_QSPICLKSOURCE_PLL;
  PeriphClkInitStruct.CkperClockSelection = RCC_CLKPSOURCE_HSE;
  PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_CLKP;
  PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
  PeriphClkInitStruct.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_LSE;
  PeriphClkInitStruct.Lptim2ClockSelection = RCC_LPTIM2CLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_PWREx_EnableUSBVoltageDetector();
}
#endif // MAX_CLOCK_ONLY

void HAL_MspInit(void)
{
  __HAL_RCC_SYSCFG_CLK_ENABLE();
}

//extern UART_HandleTypeDef huart2;
extern void MX_USART2_UART_Init(void);

extern "C" {
void HARD_Breakpoint() {
	__asm__("BKPT");
	// This is supposed to now call HARD_Breakpoint_Handler() or something...
}

void NMI_Handler(void) {
	BREAKPOINT();
	while(1);
}


#if defined(DEBUG)
		#if __CM3_CMSIS_VERSION < 0x2000
			// help GDB inspect CMSIS register definitions in core_cm3.h
			volatile NVIC_Type*          pNVIC      = NVIC;
			volatile SCB_Type*           pSCB       = SCB;
			volatile SysTick_Type*       pSysTick   = SysTick;
			volatile ITM_Type*           pITM       = ITM;
			volatile MPU_Type*           pMPU       = MPU;
			volatile CoreDebug_Type*     pCoreDebug = CoreDebug;
		#else
			// help GDB inspect CMSIS register definitions in core_cm3.h
			volatile NVIC_Type*          pNVIC      = NVIC;
			volatile SCB_Type*           pSCB       = SCB;
			volatile SysTick_Type*       pSysTick   = SysTick;
			volatile ITM_Type*           pITM       = ITM;
			volatile MPU_Type*           pMPU       = MPU;
			volatile CoreDebug_Type*     pCoreDebug = CoreDebug;
		#endif
	#endif

__attribute__((optimize("O0")))
void HardFault_HandlerC(unsigned long *hardfault_args) {
	volatile unsigned long stacked_r0 ;
  	volatile unsigned long stacked_r1 ;
  	volatile unsigned long stacked_r2 ;
  	volatile unsigned long stacked_r3 ;
  	volatile unsigned long stacked_r12 ;
  	volatile unsigned long stacked_lr ;
  	volatile unsigned long stacked_pc ;
  	volatile unsigned long stacked_psr ;
  	volatile unsigned long _CFSR ;
	volatile unsigned long _UFSR ;
  	volatile unsigned long _HFSR ;
  	volatile unsigned long _DFSR ;
  	volatile unsigned long _AFSR ;
  	volatile unsigned long _BFAR ;
  	volatile unsigned long _MMAR ;

  	stacked_r0 = ((unsigned long)hardfault_args[0]) ;
  	stacked_r1 = ((unsigned long)hardfault_args[1]) ;
  	stacked_r2 = ((unsigned long)hardfault_args[2]) ;
  	stacked_r3 = ((unsigned long)hardfault_args[3]) ;
  	stacked_r12 = ((unsigned long)hardfault_args[4]) ;
  	stacked_lr = ((unsigned long)hardfault_args[5]) ;
  	stacked_pc = ((unsigned long)hardfault_args[6]) ;
  	stacked_psr = ((unsigned long)hardfault_args[7]) ;

  	// Configurable Fault Status Register
  	// Consists of MMSR, BFSR and UFSR
  	_CFSR = (*((volatile unsigned long *)(0xE000ED28))) ;

	// Usage Fault Status Register
	_UFSR = _CFSR >> 0x10;

  	// Hard Fault Status Register
  	_HFSR = (*((volatile unsigned long *)(0xE000ED2C))) ;

  	// Debug Fault Status Register
  	_DFSR = (*((volatile unsigned long *)(0xE000ED30))) ;

  	// Auxiliary Fault Status Register
  	_AFSR = (*((volatile unsigned long *)(0xE000ED3C))) ;

  	// Read the Fault Address Registers. These may not contain valid values.
  	// Check BFARVALID/MMARVALID to see if they are valid values
  	// MemManage Fault Address Register
  	_MMAR = (*((volatile unsigned long *)(0xE000ED34))) ;
  	// Bus Fault Address Register
  	_BFAR = (*((volatile unsigned long *)(0xE000ED38))) ;
	#if defined(DEBUG)
	                                       // Possible Causes of Fault Status Registers (Yiu, The definitive guide to the ARM Cortex-M3, 2010)
	bool MMARVALID = (_MMAR & (1<<7)) > 0; // Indicates the Memory Manage Address register (MMAR) contains a valid fault addressing value.
	bool MSTKERR   = (_MMAR & (1<<4)) > 0; // Error occurred during stacking (starting of exception).
	                                       // 1. Stack pointer is corrupted.
	                                       // 2. Stack size is too large, reaching a region not defined by the MPU or disallowed in the MPU configuration.
	bool MUNSTKERR = (_MMAR & (1<<3)) > 0; // Error occurred during unstacking (ending of exception). If there was no error
	                                       // stacking but error occurred during unstacking, it might be because of the
	                                       // following reasons:
	                                       // 1. Stack pointer was corrupted during exception.
	                                       // 2. MPU configuration was changed by exception handler.
	bool DACCVIOL = (_MMAR & (1<<1)) > 0;  // Violation to memory access protection, which is defined by MPU setup.
	                                       // For example, user application trying to access privileged-only region.
	bool IACCVIOL = (_MMAR & (1<<0)) > 0;  // 1. Violation to memory access protection, which is defined by MPU setup.
	                                       // For example, user application trying to access privileged-only region.
	                                       // Stacked PC might be able to locate the code that caused the problem.
	                                       // 2. Branch to nonexecutable regions.
	                                       // 3. Invalid exception return code.
	                                       // 4. Invalid entry in exception vector table. For example, loading of an executable
	                                       // image for traditional ARM core into the memory, or exception happened before vector table was set.
	                                       // 5. Stacked PC corrupted during exception handling.

	bool BFARVALID = (_BFAR & (1<<7)) > 0; // Indicates the Bus Fault Address register contains a valid bus fault address.
	bool STKERR    = (_BFAR & (1<<4)) > 0; // Error occurred during stacking (starting of exception).
	bool UNSTKERR  = (_BFAR & (1<<3)) > 0; // Error occurred during unstacking (ending of exception. If there was no error
	                                       // stacking but error occurred during unstacking, it might be that the stack pointer was corrupted during exception.
	bool IMPRECISERR=(_BFAR & (1<<2)) > 0; // Bus error during data access. The fault address may be indicated by BFAR.
	bool PRECISERR = (_BFAR & (1<<1)) > 0; // Bus error during data access. The fault address may be inidcated by BFAR.
	bool IBUSERR   = (_BFAR & (1<<0)) > 0; // Branch to invalid memory regions, or invalid exception return code, or invalid entry in exception vector table, or stacked PC corrupted during function calls, or access to NVIC or SCB in user mode (nonprivileged).

	bool DIVBYZERO = (_UFSR & (1<<9)) > 0; // is DIV_0_TRP set? find code at fault with stacked PC
	bool UNALIGNED = (_UFSR & (1<<8)) > 0;
	bool NOCP      = (_UFSR & (1<<3)) > 0;
	bool INVPC     = (_UFSR & (1<<2)) > 0;
	bool INVSTATE  = (_UFSR & (1<<1)) > 0;
	bool UNDEFINSTR= (_UFSR & (1<<0)) > 0; // 1. Use of instructions not supported in Cortex-M3.
	                                       // 2. Bad/corrupted memory contents.
	                                       // 3. Loading of ARM object code during link stage. Checks compile steps.
	                                       // 4. Instruction align problem. for example, if GNU tool chain is used, omitting of
	                                       //    .align after .ascii might cause next instruction to be unaligned
	bool DEBUGEVF  = (_HFSR &(1<<31)) > 0;
	bool FORCED    = (_HFSR &(1<<30)) > 0; // 1. Trying to run SVC/BKPT within SVC/monitor or another handler with same or higher priority.
	                                       // 2. A hard fault occurred if the corresponding handler is disabled or cannot be started because
	                                       // another exception with the same or higher priority is running, or because another exception
	                                       // with the same or higher priority is running, or because exception mask is set.
	bool VECTBL    = (_HFSR &(1<< 1)) > 0;

	bool EXTERNAL  = (_DFSR & (1<<4)) > 0;
	bool VCATCH    = (_DFSR & (1<<3)) > 0;
	bool DWTTRAP   = (_DFSR & (1<<2)) > 0; // DWT watchpoint event has occurred.
	bool BKPT      = (_DFSR & (1<<1)) > 0; // Breakpoint instruction is executed, or FPB unit generated breakpoint event.
	bool HALTED    = (_DFSR & (1<<0)) > 0; // Halt request in NVIC

    pNVIC      = NVIC;
    pSCB       = SCB;
    pSysTick   = SysTick;
    pITM       = ITM;
    pMPU       = MPU;
    pCoreDebug = CoreDebug;
#endif // defined(DEBUG)

 	// at this point you can read data from the variables with
	// "p/x stacked_pc"
	// "info symbol <address>" should list the code line
	// "info address <FunctionName>"
	// "info registers" might help
	// "*((char *)0x00) = 5;" should create a hard-fault to test
	BREAKPOINT();
	while(1);
}

void HardFault_Handler()
{
	// This assembly code will find the location of the stack and pass it to a C function hard fault handler (HardFault_HandlerC)
		asm(
			"TST LR, #4 \n"          // Test EXC_RETURN number in LR bit 2 to determine if main stack or program stack is in use.
			"ITE EQ \n"
			"MRSEQ R0, MSP \n"
			"MRSNE R0, PSP \n"
			"B HardFault_HandlerC \n"
		);

}

void MemManage_Handler(void) {
	BREAKPOINT();
	while(1);
}

void BusFault_Handler(void) {
	BREAKPOINT();
	while(1);
}

void UsageFault_Handler(void) {
	BREAKPOINT();
	while(1);
}

void SVC_Handler(void) {
	BREAKPOINT();
	while(1);
}

void DebugMon_Handler(void) {
	BREAKPOINT();
	while(1);
}

void PendSV_Handler(void) {
	BREAKPOINT();
	while(1);
}

void SysTick_Handler() {
	HAL_IncTick();
}

// An exception or interrupt without a handler.
// Check irq_num, note that negative values are exceptions
void Unknown_Handler(void) {
	volatile uint32_t ipsr = __get_IPSR();
	volatile int irq_num = ipsr - 16;

	BREAKPOINT();
	while(1);
}

// So... processor_selector.h silently #undefs CRC
// It is stupid but they have a point. Recreate it here.
// Then #undef it again, in accordance with their wishes
#define CRC                 ((CRC_TypeDef *) CRC_BASE)

CRC_HandleTypeDef hcrc;
static void MX_CRC_Init(void) {
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
    __BKPT();
}

void HAL_CRC_MspInit(CRC_HandleTypeDef* crcHandle) {
	if(crcHandle->Instance==CRC)
		__HAL_RCC_CRC_CLK_ENABLE();
}

void HAL_CRC_MspDeInit(CRC_HandleTypeDef* crcHandle) {
	if(crcHandle->Instance==CRC)
		__HAL_RCC_CRC_CLK_DISABLE();
}
#undef CRC

void BootstrapCode() {
	SystemInit();
	//SCB->VTOR = FLASH_BANK1_BASE | VECT_TAB_OFFSET;       /* Vector Table Relocation in Internal FLASH */
	//SCB->VTOR = FLASH_BANK1_BASE; // Vector table in flash, add offset later (must for TinyCLR with new flash base)

	//PrepareImageRegions(); // startup asm now does this, I think

	SCB->VTOR = (UINT32)&Load$$ER_FLASH$$Base;
	__DSB();
	__ISB();

	SCB_EnableICache();
	SCB_EnableDCache();

	HAL_Init(); // Later calls HAL_MspInit()
#ifdef MAX_CLOCK_ONLY
	MaxSystemClock_Config();
#else
	MinSystemClock_Config();
#endif
	//MX_USART2_UART_Init();
	#ifdef DEBUG
	__HAL_DBGMCU_FREEZE_TIM2();
	__HAL_DBGMCU_FREEZE_TIM5();
	__HAL_DBGMCU_FREEZE_LPTIM1();
	__HAL_DBGMCU_FREEZE_LPTIM2();
	#endif
	MX_CRC_Init();
	START_UP_DELAY();
}
} // extern "C"

