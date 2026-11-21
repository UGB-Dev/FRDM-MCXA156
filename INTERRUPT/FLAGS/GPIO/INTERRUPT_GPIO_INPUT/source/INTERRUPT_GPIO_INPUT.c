/*
 * AUTOR 		: UGB_Dev
 * DATE  		: 19/11/2026
 * VERSION 		: V1.0.0
 * DESCRIPTION	: INTERRUPCION GPIO; Se configura al MCU para capturar eventos de interrupcion
 *                mediante sus banderas.
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

	MRCC0 -> MRCC_GLB_CC1 |= MRCC_MRCC_GLB_CC1_GPIO1_MASK |// HABILITA RELOJ GRUPO 1
							 MRCC_MRCC_GLB_CC1_PORT1_MASK;

	MRCC0 -> MRCC_GLB_RST1 |= MRCC_MRCC_GLB_RST1_GPIO1_MASK |
							  MRCC_MRCC_GLB_RST1_PORT1_MASK; // SALE DE MODO RESET

	SYSCON -> CLKUNLOCK |= SYSCON_CLKUNLOCK_UNLOCK_MASK; // BLOQUEO DE PERIFERICO MRCC

	/* PASO 2: CONFIGURARACION DE LOS PINES COMO ENTRADA Y/O SALIDA */
	PORT1 -> PCR[0] |= PORT_PCR_PE_MASK  | // HABILITACION DE RESISTENCIA EN PULL
					   PORT_PCR_IBE_MASK | // HABILITACION DE BUFFER DE ENTRADA (NECESARIO PARA ENTRADAS)
					   PORT_PCR_PS_MASK  ; // SELECCIONA LA RESISTENCIA DE PULL-UP

	GPIO1 -> PDDR &= ~GPIO_PDDR_PDD0_MASK; // GPIO1_0 COMO ENTRADA ; POR DEFECTO NO ES NECESARIO ESTA LINEA
	GPIO1 -> ICR[0] |= 0xA<<GPIO_ICR_IRQC_SHIFT ; // HABILITA ISF E INTERRUPCION POR FLANCO DE BAJADA

	while(1) {
		if (GPIO1 -> ISFR[0] & GPIO_ISFR_ISF0_MASK ){
			PRINTF("BANDERA DE INTERRUPCION ACTIVA EN GPIO1_0 .........  \r\n");
			GPIO1 -> ISFR[0] |= GPIO_ISFR_ISF0_MASK; // BORRA BANDERA DE INTERRUPCION ESCRIBIENDO UNO LOGICO
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
