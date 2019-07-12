#include <tinyhal.h>
#include <stm32h7xx_hal.h>
#include "..\stm32h7xx.h"

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

/**
  * @brief  CPU L1-Cache enable.
  * @param  None
  * @retval None
  */
static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_InvalidateICache();
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_InvalidateDCache();
  SCB_EnableDCache();
}

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

// DEFAULT 96 MHZ FROM HSE BYPASS (DEV BOARD ONLY)
static void DefaultSystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

	MODIFY_REG(PWR->CR3, PWR_CR3_SCUEN, 0);

	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

	while ((PWR->D3CR & (PWR_D3CR_VOSRDY)) != PWR_D3CR_VOSRDY)
	{

	}

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 1;
	RCC_OscInitStruct.PLL.PLLN = 24;
	RCC_OscInitStruct.PLL.PLLP = 2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	RCC_OscInitStruct.PLL.PLLR = 2;
	RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
	RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
	RCC_OscInitStruct.PLL.PLLFRACN = 0;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
									|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
									|RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
	RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
	{
		Error_Handler();
	}
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART3|RCC_PERIPHCLK_USB;
	PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
	PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
	{
		Error_Handler();
	}
}

void HAL_MspInit(void)
{
  __HAL_RCC_SYSCFG_CLK_ENABLE();
}

extern "C" {
void HARD_Breakpoint() {
	__asm__("BKPT");
	// This is supposed to now call HARD_Breakpoint_Handler() or something...
}

void NMI_Handler(void) {
	BREAKPOINT();
	while(1);
}

void HardFault_Handler(void) {
	BREAKPOINT();
	while(1);
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

void BootstrapCode() {
	SystemInit();
	//SCB->VTOR = FLASH_BANK1_BASE | VECT_TAB_OFFSET;       /* Vector Table Relocation in Internal FLASH */
	//SCB->VTOR = FLASH_BANK1_BASE; // Vector table in flash, add offset later (must for TinyCLR with new flash base)

	__DSB();
	SCB->VTOR = (UINT32)&Load$$ER_FLASH$$Base;
	__ISB();
	//PrepareImageRegions(); // startup asm now does this, I think
	//CPU_CACHE_Enable(); // Turn on later after we get out of our debugging hole
	HAL_Init(); // Later calls HAL_MspInit()
	DefaultSystemClock_Config(); // 96 MHz
	#ifdef DEBUG
	__HAL_DBGMCU_FREEZE_TIM2();
	#endif
}
} // extern "C"

