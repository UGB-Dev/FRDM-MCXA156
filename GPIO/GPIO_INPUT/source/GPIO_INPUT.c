/*
 * AUTOR 		: UGB_Dev
 * DATE  		: 18/11/2026
 * VERSION 		: V1.0.0
 * DESCRIPTION	: GPIO_INPUT; Lectura en pin P1_0 para generar cambio logico en el pin P3_0
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

	GPIO3 -> PDDR |= GPIO_PDDR_PDD0_MASK; // GPIO3_0 COMO SALIDA
	GPIO3 -> PDOR |= GPIO_PDOR_PDO0_MASK; // LED AZUL OFF

	PRINTF("CONFIGURACION DEL PERIFERICO GPIO1 Y GPIO3 FINALIZADA ...... \r\n");

	while(1) {
		if(!(GPIO1 -> PDIR & GPIO_PDIR_PDI0_MASK)){
			GPIO3 -> PDOR &= ~GPIO_PDOR_PDO0_MASK;
			PRINTF("LED AZUL ON \r\n");
			Delay_MS(100);
		}
		else {
			GPIO3 -> PDOR |= GPIO_PDOR_PDO0_MASK;
			PRINTF("LED AZUL OFF \r\n");
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
