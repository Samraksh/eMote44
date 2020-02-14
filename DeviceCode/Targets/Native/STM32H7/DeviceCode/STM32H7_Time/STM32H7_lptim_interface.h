#ifndef STM32H7_LPTIM_INTERFACE_H
#define STM32H7_LPTIM_INTERFACE_H


#ifdef __cplusplus
extern "C" {
#endif

void LptimInit(void);
int callLptimSetCompareMicroseconds(uint32_t ticks);
int callLptimSetCompare(uint16_t ticks, bool is_next_epoch);
void lptimIRQHandler(void);
uint64_t requestLptimCounter(void);

#ifdef __cplusplus
}
#endif



#endif // STM32H7_LPTIM_INTERFACE_H

