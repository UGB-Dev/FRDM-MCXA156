/*
 * AUTOR 		: UGB_Dev
 * DATE  		: 19/11/2026
 * VERSION 		: V1.0.0
 * DESCRIPTION	: INTERRUPT DISPLAY COUNTER; Conteo de 0 a 99 usando el handler como interrupcion por flanco de bajada en
 *                GPIO1_0 y GPIO3_28, visualizando el valor en dos display de 7 segmentos con multiplexacion.
 */

#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"

#define Q1_H	(GPIO1 -> PDOR |= GPIO_PDOR_PDO8_MASK)
#define Q1_L	(GPIO1 -> PDOR &= ~GPIO_PDOR_PDO8_MASK)
#define Q2_H	(GPIO3 -> PDOR |= GPIO_PDOR_PDO22_MASK)
#define Q2_L	(GPIO3 -> PDOR &= ~GPIO_PDOR_PDO22_MASK)
#define A_H		(GPIO2 -> PDOR |= GPIO_PDOR_PDO17_MASK)
#define A_L		(GPIO2 -> PDOR &= ~GPIO_PDOR_PDO17_MASK)
#define B_H		(GPIO2 -> PDOR |= GPIO_PDOR_PDO23_MASK)
#define B_L		(GPIO2 -> PDOR &= ~GPIO_PDOR_PDO23_MASK)
#define C_H		(GPIO2 -> PDOR |= GPIO_PDOR_PDO21_MASK)
#define C_L		(GPIO2 -> PDOR &= ~GPIO_PDOR_PDO21_MASK)
#define D_H		(GPIO0 -> PDOR |= GPIO_PDOR_PDO17_MASK)
#define D_L		(GPIO0 -> PDOR &= ~GPIO_PDOR_PDO17_MASK)
#define E_H		(GPIO0 -> PDOR |= GPIO_PDOR_PDO19_MASK)
#define E_L		(GPIO0 -> PDOR &= ~GPIO_PDOR_PDO19_MASK)
#define F_H		(GPIO0 -> PDOR |= GPIO_PDOR_PDO21_MASK)
#define F_L		(GPIO0 -> PDOR &= ~GPIO_PDOR_PDO21_MASK)
#define G_H		(GPIO0 -> PDOR |= GPIO_PDOR_PDO23_MASK)
#define G_L		(GPIO0 -> PDOR &= ~GPIO_PDOR_PDO23_MASK)

#define CERO	A_H; B_H; C_H; D_H; E_H; F_H; G_L;
#define UNO 	A_L; B_H; C_H; D_L; E_L; F_L; G_L;
#define DOS 	A_H; B_H; C_L; D_H; E_H; F_L; G_H;
#define TRES 	A_H; B_H; C_H; D_H; E_L; F_L; G_H;
#define CUATRO 	A_L; B_H; C_H; D_L; E_L; F_H; G_H;
#define CINCO 	A_H; B_L; C_H; D_H; E_L; F_H; G_H;
#define SEIS 	A_H; B_L; C_H; D_H; E_H; F_H; G_H;
#define SIETE 	A_H; B_H; C_H; D_L; E_L; F_L; G_L;
#define OCHO 	A_H; B_H; C_H; D_H; E_H; F_H; G_H;
#define NUEVE 	A_H; B_H; C_H; D_L; E_L; F_H; G_H;
#define Unidad  0
#define Decena  1

uint8_t Tick_us = 0;
uint32_t Tick_ms = 0;
int count=0;

void Delay_Init(void);
void Delay_MS(uint32_t ms);
void Imprimir_Numero(uint8_t Num, uint8_t UD);

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

	MRCC0 -> MRCC_GLB_CC1 |= MRCC_MRCC_GLB_CC1_GPIO0_MASK |
			                 MRCC_MRCC_GLB_CC1_GPIO1_MASK |
							 MRCC_MRCC_GLB_CC1_GPIO2_MASK |
							 MRCC_MRCC_GLB_CC1_GPIO3_MASK |// HABILITA RELOJ GRUPO 0, 1, 2 Y 3
							 MRCC_MRCC_GLB_CC1_PORT1_MASK |
							 MRCC_MRCC_GLB_CC1_PORT3_MASK;

	MRCC0 -> MRCC_GLB_RST1 |= MRCC_MRCC_GLB_RST1_GPIO0_MASK |
							  MRCC_MRCC_GLB_RST1_GPIO1_MASK |
							  MRCC_MRCC_GLB_RST1_GPIO2_MASK |
							  MRCC_MRCC_GLB_RST1_GPIO3_MASK |
							  MRCC_MRCC_GLB_RST1_PORT1_MASK |
							  MRCC_MRCC_GLB_RST1_PORT3_MASK; // SALE DE MODO RESET

	SYSCON -> CLKUNLOCK |= SYSCON_CLKUNLOCK_UNLOCK_MASK; // BLOQUEO DE PERIFERICO MRCC

    /* PASO 2: CONFIGURARACION DE LOS PINES COMO ENTRADA Y/O SALIDA */
	PORT1 -> PCR[0] |= PORT_PCR_PE_MASK  | // HABILITACION DE RESISTENCIA EN PULL
					   PORT_PCR_IBE_MASK | // HABILITACION DE BUFFER DE ENTRADA (NECESARIO PARA ENTRADAS)
					   PORT_PCR_PS_MASK  ; // SELECCIONA LA RESISTENCIA DE PULL-UP

	GPIO1 -> PDDR &= ~GPIO_PDDR_PDD0_MASK; // GPIO1_0 COMO ENTRADA ; POR DEFECTO NO ES NECESARIO ESTA LINEA
	GPIO1 -> ICR[0] |= 0xA<<GPIO_ICR_IRQC_SHIFT; // HABILITA ISF E INTERRUPCION POR FLANCO DE BAJADA

	PORT3 -> PCR[28] |= PORT_PCR_PE_MASK  | // HABILITACION DE RESISTENCIA EN PULL
					    PORT_PCR_IBE_MASK | // HABILITACION DE BUFFER DE ENTRADA (NECESARIO PARA ENTRADAS)
					    PORT_PCR_PS_MASK  ; // SELECCIONA LA RESISTENCIA DE PULL-UP

	GPIO3 -> PDDR &= ~GPIO_PDDR_PDD28_MASK; // GPIO3_28 COMO ENTRADA ; POR DEFECTO NO ES NECESARIO ESTA LINEA
	GPIO3 -> ICR[28] |= 0xA<<GPIO_ICR_IRQC_SHIFT ; // HABILITA ISF E INTERRUPCION POR FLANCO DE BAJADA


   	GPIO0 -> PDDR |= GPIO_PDDR_PDD17_MASK | GPIO_PDDR_PDD19_MASK | GPIO_PDDR_PDD21_MASK | GPIO_PDDR_PDD23_MASK; // GPIO0_17, GPIO0_19, GPIO0_21 Y GPIO0_23 COMO SALIDA
   	GPIO1 -> PDDR |= GPIO_PDDR_PDD8_MASK; // GPIO1_8
   	GPIO2 -> PDDR |= GPIO_PDDR_PDD17_MASK | GPIO_PDDR_PDD21_MASK | GPIO_PDDR_PDD23_MASK ; // GPIO2_17, GPIO2_21 Y GPIO2_23 COMO SALIDA
	GPIO3 -> PDDR |= GPIO_PDDR_PDD22_MASK; // GPIO3_22 COMO SALIDA


	/* PASO 3: HABILITACION DE VECTOR DE INTERRUPCION */
	NVIC -> ISER[GPIO1_IRQn/32] |= 1<<(GPIO1_IRQn % 32); // HABILITA INTERRUPCION GLOBAL GPIO1
	NVIC -> ISER[GPIO3_IRQn/32] |= 1<<(GPIO3_IRQn % 32); // HABILITA INTERRUPCION GLOBAL GPIO3


	PRINTF("CONFIGURACION DEL PERIFERICOS FINALIZADA ...... \r\n");

	uint8_t U=0, D=0;

    while(1) {
    	U = count%10;
    	D = count/10;

		for(uint8_t i=0; i<10; i++){ // MULTIPLEXEA EN 160 ms
			Imprimir_Numero(U, Unidad);
			Imprimir_Numero(D, Decena);
		}
    }
}

void Imprimir_Numero(uint8_t Num, uint8_t UD){
	switch(Num){
		case 0:		CERO;	break;
		case 1:		UNO;	break;
		case 2:		DOS;	break;
		case 3:		TRES;	break;
		case 4:		CUATRO;	break;
		case 5:		CINCO;	break;
		case 6:		SEIS;	break;
		case 7:		SIETE;	break;
		case 8:		OCHO;	break;
		case 9:		NUEVE;	break;
	}

	if(UD == Unidad){ Q1_L; Q2_H;}
	else{ Q1_H; Q2_L;}
	Delay_MS(8);
}

void GPIO1_DriverIRQHandler(void){
	if (GPIO1 -> ISFR[0] & GPIO_ISFR_ISF0_MASK ){
		count++;
		if(count > 99) count = 0;
		GPIO1 -> ISFR[0] |= GPIO_ISFR_ISF0_MASK; // BORRA BANDERA DE INTERRUPCION ESCRIBIENDO UNO LOGICO
	}
}

void GPIO3_DriverIRQHandler(void){
	if (GPIO3 -> ISFR[0] & GPIO_ISFR_ISF28_MASK ){
		count--;
		if(count < 0) count = 99;
		GPIO3 -> ISFR[0] |= GPIO_ISFR_ISF28_MASK; // BORRA BANDERA DE INTERRUPCION ESCRIBIENDO UNO LOGICO
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
		SysTick -> LOAD = Tick_ms; // 96MHz/Fbase = 96MHz/1000Hz = 96K Tick = 96K Tick/96MHz = 1ms por cada Tick
		SysTick -> VAL = 0;
		SysTick -> CTRL |= SysTick_CTRL_ENABLE_Msk; // HABILITA SYSTICK
		while(!(SysTick -> CTRL & SysTick_CTRL_COUNTFLAG_Msk)); // ESPERA HASTA QUE VAL SEA CERO
		SysTick -> CTRL &= ~SysTick_CTRL_ENABLE_Msk; // DESHABILITA SYSTICK
	}
}
