/**
 * @file UART.c
 * @brief Implementación del driver de UART para Kinetis.
 *
 * Incluye inicialización de UARTs, configuración de baudrate y paridad,
 * y funciones básicas para envío y recepción de datos individuales.
 */

#include "hardware.h"
#include "UART.h"

/**
 * @brief Inicializa el UART con paridad especificada.
 *
 * @param parity Paridad (ver uart_parity_t).
 */
void UART_Init (char parity)
{

// Note: 5.6 Clock Gating page 192
// Any bus access to a peripheral that has its clock disabled generates an error termination.
	    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;

	    SIM->SCGC4 |= SIM_SCGC4_UART0_MASK;
		SIM->SCGC4 |= SIM_SCGC4_UART1_MASK;
		SIM->SCGC4 |= SIM_SCGC4_UART2_MASK;
		SIM->SCGC4 |= SIM_SCGC4_UART3_MASK;
		SIM->SCGC1 |= SIM_SCGC1_UART4_MASK;
		SIM->SCGC1 |= SIM_SCGC1_UART5_MASK;

		NVIC_EnableIRQ(UART0_RX_TX_IRQn);
		NVIC_EnableIRQ(UART1_RX_TX_IRQn);
		NVIC_EnableIRQ(UART2_RX_TX_IRQn);
		NVIC_EnableIRQ(UART3_RX_TX_IRQn);
		NVIC_EnableIRQ(UART4_RX_TX_IRQn);
		NVIC_EnableIRQ(UART5_RX_TX_IRQn);

		//Configure UART0 TX and RX PINS

		PORTB->PCR[UART0_TX_PIN]=0x0; //Clear all bits
		PORTB->PCR[UART0_TX_PIN]|=PORT_PCR_MUX(PORT_mAlt3); 	 //Set MUX to UART0
		PORTB->PCR[UART0_TX_PIN]|=PORT_PCR_IRQC(PORT_eDisabled); //Disable interrupts
//----------------------------------------------------------------------------------
		PORTB->PCR[UART0_RX_PIN]=0x0; //Clear all bits
		PORTB->PCR[UART0_RX_PIN]|=PORT_PCR_MUX(PORT_mAlt3); 	 //Set MUX to UART0
		PORTB->PCR[UART0_RX_PIN]|=PORT_PCR_IRQC(PORT_eDisabled); //Disable interrupts


	//UART0 Baudrate Setup + Parity Setup (8o1) (8 bits, odd parity, 1 stop bit)

		UART_SetBaudRate(UART0, UART_HAL_DEFAULT_BAUDRATE);
		UART_SetParity(UART0, parity);

	//Enable UART0 Xmiter and Rcvr

	UART0->C2=UART_C2_TE_MASK | UART_C2_RE_MASK;

}

/**
 * @brief Configura la paridad para un UART específico.
 *
 * @param uart UART a configurar.
 * @param parity Tipo de paridad.
 */
void UART_SetParity(UART_Type *uart, uart_parity_t parity) {
    /* poner formato 9‑bit si se usa paridad */
    if (parity == UART_PARITY_NONE) {
        uart->C1 &= ~(UART_C1_M_MASK | UART_C1_PE_MASK | UART_C1_PT_MASK); // 8N
    } else {
        uart->C1 |= UART_C1_M_MASK;  // M=1 → 9 bits (8 datos + paridad)
        uart->C1 &= ~(UART_C1_PE_MASK | UART_C1_PT_MASK);
        uart->C1 |= UART_C1_PE_MASK;                 // PE=1
        if (parity == UART_PARITY_ODD) uart->C1 |= UART_C1_PT_MASK; // PT=1 para impar
    }
    uart->BDH &= ~UART_BDH_SBNS_MASK; // 1 stop
}

/**
 * @brief Configura el baudrate para un UART específico.
 *
 * @param uart UART a configurar.
 * @param baudrate Baudrate deseado.
 */
void UART_SetBaudRate (UART_Type *uart, uint32_t baudrate)
{
	uint16_t sbr, brfa;
	uint32_t clock;

	clock = ((uart == UART0) || (uart == UART1))?(__CORE_CLOCK__):(__CORE_CLOCK__ >> 1);

	baudrate = ((baudrate == 0)?(UART_HAL_DEFAULT_BAUDRATE):
			((baudrate > 0x1FFF)?(UART_HAL_DEFAULT_BAUDRATE):(baudrate)));

	sbr = clock / (baudrate << 4);               // sbr = clock/(Baudrate x 16)
	brfa = (clock << 1) / baudrate - (sbr << 5); // brfa = 2*Clock/baudrate - 32*sbr

	uart->BDH = UART_BDH_SBR(sbr >> 8);
	uart->BDL = UART_BDL_SBR(sbr);
	uart->C4 = (uart->C4 & ~UART_C4_BRFA_MASK) | UART_C4_BRFA(brfa);
}

/**
 * @brief Envía un byte de datos por UART0.
 *
 * @param txdata Byte a enviar.
 */
void UART_Send_Data(unsigned char txdata)
{
	if(((UART0->S1) & UART_S1_TDRE_MASK) != 0)
	{
		UART0->D = txdata;
	}
}

/**
 * @brief Recibe un byte de datos por UART0.
 *
 * @return Byte recibido.
 */
unsigned char UART_Recieve_Data(void)
{
    uint8_t s1 = UART0->S1;
    if (s1 & UART_S1_RDRF_MASK) {
        unsigned char d = UART0->D;            // leer SIEMPRE para limpiar flags
        // if (s1 & UART_S1_PF_MASK) {
        //     return 0;                    // descartar el byte con paridad mala
        // }
        return d;                        // byte válido
    }
    return 0;                            // no hay dato disponible
}
