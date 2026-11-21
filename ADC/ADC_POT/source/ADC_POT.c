/*
 * AUTOR 		: UGB_Dev
 * DATE  		: 19/11/2026
 * VERSION 		: V1.0.0
 * DESCRIPTION	: ADC_POT; Captura de voltaje en la salida del potenciometro
 *                ubicado en el pin GPIO2_0 usando el ADC canal A_0.
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
uint16_t Capture_ADC(void);
uint16_t adc_value0=0, adc_value1=0;

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

	MRCC0 -> MRCC_GLB_CC1 |= MRCC_MRCC_GLB_CC1_GPIO2_MASK | // HABILITA RELOJ GRUPO 2
							 MRCC_MRCC_GLB_CC1_ADC0_MASK |  // HABILITA RELOJ ADC0
							 MRCC_MRCC_GLB_CC1_PORT2_MASK;

	MRCC0 -> MRCC_GLB_RST1 |= MRCC_MRCC_GLB_RST1_GPIO2_MASK |
							  MRCC_MRCC_GLB_RST1_ADC0_MASK |
							  MRCC_MRCC_GLB_RST1_PORT2_MASK; // SALE DE MODO RESET

	MRCC0 -> MRCC_ADC0_CLKSEL = 0x1; // FUENTE DE RELOJ POR FRO (96 MHz) PARA EL ADC 0
	MRCC0 -> MRCC_ADC0_CLKDIV &= ~MRCC_MRCC_ADC0_CLKDIV_HALT_MASK; // BORRA BIT HALT ( EVITA BLOQUEO EN MICROCCONTROLADOR)
	MRCC0 -> MRCC_ADC0_CLKDIV |=1;

	SYSCON -> CLKUNLOCK |= SYSCON_CLKUNLOCK_UNLOCK_MASK; // BLOQUEO DE PERIFERICO MRCC

	/* PASO 2: CONFIGURACION DEL GPIO Y PORT */
	GPIO2 -> PDDR &= ~GPIO_PDDR_PDD0_MASK; // PIN GPIO2_0 COMO ENTRADA ADC
	PORT2 -> PCR[0] = 0; // PIN GPIO2_0 COMO ADC ( MODO ALTERNATIVO ALT0 = GPIO = ADC)

	/* PASO 3: CONFIGURACION DEL ADC */
	ADC0 -> CMD[0].CMDL |= ADC_CMDL_MODE_MASK; // RESOLUCION DE 16 bits
	ADC0 -> CMD[0].CMDL |= 0<<ADC_CMDL_ADCH_SHIFT; // SELECCION DEL CANAL A_0
	ADC0 -> TCTRL[0] |= 1<<ADC_TCTRL_TCMD_SHIFT; // SELECCIONA EL CMD1 (CMD[0]) PARA CONVERSION ADC
	//ADC0 -> CFG |= 2<<ADC_CFG_REFSEL_SHIFT; // SELECCION DE VREF COMO VDDA_ANA
	ADC0 -> CTRL |= ADC_CTRL_ADCEN_MASK; // HABILITA EL ADC

	float Value_VDD = 0;
	uint16_t Entero=0, Flotante=0;

	while(1) {
		Value_VDD = (Capture_ADC()*3.3)/65536;
		Entero = Value_VDD;
		Flotante = (Value_VDD - Entero)*1000;
		PRINTF("Vout Potenciometro= %d.%d \n", Entero, Flotante);
		Delay_MS(80);
	}
}


uint16_t Capture_ADC(void){
	ADC0 -> SWTRIG |= ADC_SWTRIG_SWT0_MASK; // SE GENERA UNA PETICION DE CONVERSION ADC
	while((ADC0 -> FCTRL & ADC_FCTRL_FCOUNT_MASK) == 0); // ESPERA HASTA DETECTAR UN CONTEO DE CONVERSION
	return( ADC0 -> RESFIFO & 0xFFFF);
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
