/*
 * AUTOR 		: UGB_Dev
 * DATE  		: 19/11/2026
 * VERSION 		: V1.0.0
 * DESCRIPTION	: CTIMER SERVO; Se configura el periferico CTIMER0 como base de tiempo para generar
 *                PWM a 50 Hz como senal del servo .
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
void Set_Duty_PWM(uint16_t D);

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

	MRCC0 -> MRCC_GLB_CC0 |= MRCC_MRCC_GLB_CC0_CTIMER0_MASK; // HABILITA RELOJ CTIMER0

	MRCC0 -> MRCC_GLB_CC1 |= MRCC_MRCC_GLB_CC1_GPIO1_MASK | // HABILITA RELOJ GRUPO 1
							 MRCC_MRCC_GLB_CC1_PORT1_MASK;

	MRCC0 -> MRCC_GLB_RST0 |= MRCC_MRCC_GLB_RST0_CTIMER0_MASK; // SALE DE MODO RESET

	MRCC0 -> MRCC_GLB_RST1 |= MRCC_MRCC_GLB_RST1_GPIO1_MASK |
							  MRCC_MRCC_GLB_RST1_PORT1_MASK; // SALE DE MODO RESET

	MRCC0 -> MRCC_CTIMER0_CLKSEL = 0x1; // FUENTE DE RELOJ POR FRO (96 MHz) PARA EL TIMER 0
	MRCC0 -> MRCC_CTIMER0_CLKDIV &= ~MRCC_MRCC_CTIMER0_CLKDIV_HALT_MASK; // BORRA BIT HALT ( EVITA BLOQUEO EN MICROCCONTROLADOR)

	SYSCON -> CLKUNLOCK |= SYSCON_CLKUNLOCK_UNLOCK_MASK; // BLOQUEO DE PERIFERICO MRCC

	/* PASO 2: CONFIGURACION DE PIN COMO SALIDA */
	PORT1 -> PCR[8]  |=  0x5 << PORT_PCR_MUX_SHIFT; // PIN GPIO1_8 COMO SALIDA PWM ALTERN FUNCTION ALT5

	/* PASO 3: CONFIGURACION DEL TIMER */
	CTIMER0 -> MCR |= CTIMER_MCR_MR3R_MASK; // RESET TIMER EN CANAL 3 (CANAL 3 COMO PERIODO DEL PWM)
	CTIMER0 -> PWMC |= CTIMER_PWMC_PWMEN2_MASK; // PWM EN CANAL 2 (CANAL 2 COMO DUTY CYCLE)
	CTIMER0 -> PR = (SystemCoreClock / 1000000) -1; // PR = (96 MHz / 1 MHz)-1 = 95 TICKS PARA UNA BASE DE TIEMPO DE 1us
	CTIMER0 -> MR[3] = 20000 ; // 20000 * 1usbase = 20 ms PARA PERIODO (T)
	CTIMER0 -> MR[2] = 20000 - 1500 ; // 1500 * 1usbase = 1.5 ms PARA DUTY CYCLE (D)

	CTIMER0 -> TCR |= CTIMER_TCR_CEN_MASK; // HABILITA EL CONTADOR

	uint16_t grado=0;

	while(1) {
		Set_Duty_PWM(500 + (grado*2000)/180);
		Delay_MS(5);
		grado++;
		if(grado>180){
			grado=0;
			Set_Duty_PWM(500 + (grado*2000)/180);
			Delay_MS(500);
		}
	}
}

void Set_Duty_PWM(uint16_t D){
	CTIMER0 -> MR[2] = 20000 - D ; // PARA DUTY CYCLE (D) DE 0.5 A 2.5 ms
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
