/*
 * AUTOR 		: UGB_Dev
 * DATE  		: 18/11/2026
 * VERSION 		: V1.0.0
 * DESCRIPTION	: BINARY COUNTER; Contador de pulsos los cuales se observaran
 *                en los leds conectados desde GPIO3_12 a GPIO3_19.
 */

#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"

uint8_t Tick_us = 0;
uint32_t Tick_ms=0;

void Delay_Init(void);
void Delay_MS(uint32_t ms);

int main(void) {

    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    /* Init FSL debug console. */
    BOARD_InitDebugConsole();
#endif

    Delay_Init();

    /* PASO 1: ACTIVACION DEL RELOJ PARA EL PERIFERICO */
	SYSCON -> CLKUNLOCK &= ~SYSCON_CLKUNLOCK_UNLOCK_MASK; // DESBLOQUEO DE PERIFERICO MRCC

	MRCC0 -> MRCC_GLB_CC1 |= MRCC_MRCC_GLB_CC1_GPIO3_MASK |
							 MRCC_MRCC_GLB_CC1_GPIO1_MASK |
							 MRCC_MRCC_GLB_CC1_PORT1_MASK; // HABILITA RELOJ GRUPO 1 Y 3

	MRCC0 -> MRCC_GLB_RST1 |= MRCC_MRCC_GLB_RST1_GPIO3_MASK |
							  MRCC_MRCC_GLB_RST1_GPIO1_MASK |
							  MRCC_MRCC_GLB_RST1_PORT1_MASK; // SALE DE MODO RESET

	SYSCON -> CLKUNLOCK |= SYSCON_CLKUNLOCK_UNLOCK_MASK; // BLOQUEO DE PERIFERICO MRCC

	/* PASO 2: CONFIGURARACION DE LOS PINES COMO ENTRADA Y/O SALIDA */
	PORT1 -> PCR[0] |= PORT_PCR_PE_MASK | PORT_PCR_IBE_MASK | PORT_PCR_PS_MASK; // PIN P1_0 COMO ENTRADA CON RESISTENCIA PULL-UP
	GPIO1 -> PDDR &= ~GPIO_PDDR_PDD0_MASK; // GPIO1_0 COMO ENTRADA ; POR DEFECTO NO ES NECESARIO ESTA LINEA

	GPIO3 -> PDDR |= GPIO_PDDR_PDD12_MASK | GPIO_PDDR_PDD13_MASK | GPIO_PDDR_PDD14_MASK |
					 GPIO_PDDR_PDD15_MASK | GPIO_PDDR_PDD16_MASK | GPIO_PDDR_PDD17_MASK |
					 GPIO_PDDR_PDD18_MASK | GPIO_PDDR_PDD19_MASK;

	PRINTF("CONFIGURACION DEL PERIFERICOS FINALIZADA ...... \r\n");

	uint8_t count=0;

	while(1) {
		GPIO3 -> PDOR = count<<12; // DESPLAZAMIENTO PARA INICIAR DESDE PIN P3_12
		PRINTF("Count = %i\r\n", count);
		Delay_MS(100);

		if(!(GPIO1 -> PDIR & GPIO_PDIR_PDI0_MASK)){
			count ++;
			Delay_MS(100);
		}
	}
}

/* FUNCIONES PARA GENERAR DELAY POR SYSTICK*/
void Delay_Init(void){
	SysTick -> CTRL = SysTick_CTRL_CLKSOURCE_Msk; // FUENTE DE RELOJ PRINCIPAL (MAIN = 96 MHz)

	Tick_us = SystemCoreClock / 1000000;
	Tick_ms = Tick_us*1000 -1;
}

void Delay_MS(uint32_t ms){
	for( ; ms>0; ms--){ 
		SysTick -> LOAD = Tick_ms; // 96MHz/Fbase = 96MHz/1000 = 96K Tick = 96K Tick/96MHz = 1ms por cada Tick
		SysTick -> VAL = 0;
		SysTick -> CTRL |= SysTick_CTRL_ENABLE_Msk; // HABILITA SYSTICK
		while(!(SysTick -> CTRL & SysTick_CTRL_COUNTFLAG_Msk)); // ESPERA HASTA QUE VAL SEA CERO
		SysTick -> CTRL &= ~SysTick_CTRL_ENABLE_Msk; // DESHABILITA SYSTICK
	}
}
