/*
 * AUTOR 		: UGB_Dev
 * DATE  		: 17/11/2026
 * VERSION 		: V1.0.0
 * DESCRIPTION	: HELLO WORLD; Cambios logicos en pines 0, 12 y 13 de GPIO3.
 */

#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"

#define Led_Rojo_Out	GPIO_PDDR_PDD12_MASK
#define Led_Rojo_H		GPIO_PDOR_PDO12_MASK
#define Led_Azul_Out	GPIO_PDDR_PDD0_MASK
#define Led_Azul_H		GPIO_PDOR_PDO0_MASK
#define Led_Verde_Out	GPIO_PDDR_PDD13_MASK
#define Led_Verde_H		GPIO_PDOR_PDO13_MASK

uint8_t Tick_us = 0;
uint32_t Tick_ms=0;

void Delay_Init(void);
void Delay_MS(uint32_t ms);

int main(void) {
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
    MRCC0 -> MRCC_GLB_CC1 |= MRCC_MRCC_GLB_CC1_GPIO3_MASK; // HABILITA RELOJ GRUPO 3
    MRCC0 -> MRCC_GLB_RST1 |= MRCC_MRCC_GLB_RST1_GPIO3_MASK; // SALE DE MODO RESET
    SYSCON -> CLKUNLOCK |= SYSCON_CLKUNLOCK_UNLOCK_MASK; // BLOQUEO DE PERIFERICO MRCC

	/* PASO 2: CONFIGURARACION DE LOS PINES COMO SALIDA */
    GPIO3 -> PDDR |= Led_Rojo_Out | Led_Azul_Out | Led_Verde_Out; // PIN P3_0, P3_12 y P3_13
    GPIO3 -> PDOR |= Led_Rojo_H | Led_Azul_H | Led_Verde_H; // SALIDA EN 1 LOGICO EN PIN P3_0, P3_12 y P3_13 (TODOS LOS LEDS APAGADOS)

    while(1) {
    	GPIO3 -> PDOR &= ~Led_Rojo_H;
    	PRINTF("LED ROJO ON\r\n");
    	Delay_MS(500);
    	GPIO3 -> PDOR |= Led_Rojo_H;
    	PRINTF("LED ROJO OFF\r\n");
    	Delay_MS(500);

    	GPIO3 -> PDOR &= ~Led_Azul_H;
    	PRINTF("LED AZUL ON\r\n");
    	Delay_MS(500);
    	GPIO3 -> PDOR |= Led_Azul_H;
    	PRINTF("LED AZUL OFF\r\n");
    	Delay_MS(500);

    	GPIO3 -> PDOR &= ~Led_Verde_H;
    	PRINTF("LED VERDE ON\r\n");
    	Delay_MS(500);
    	GPIO3 -> PDOR |= Led_Verde_H;
    	PRINTF("LED VERDE OFF\r\n");
    	Delay_MS(500);
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
