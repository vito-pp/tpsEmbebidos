/**
 * @file DAC.c
 * @brief Driver mínimo de DAC para Kinetis (DAC0/DAC1): inicialización y escritura de dato.
 *
 * Habilita clock y configura referencia/trigger por software.
 */

#include "DAC.h"

#define DAC_DATL_DATA0_WIDTH 8

/**
 * @brief Inicializa DAC0 y DAC1.
 *
 * Habilita los clocks para DAC0 y DAC1 en SIM_SCGC2, y configura los registros C0
 * para habilitar el convertidor, seleccionar referencia VDDA y trigger por software.
 */
void DAC_Init (void)
{
	SIM->SCGC2 |= SIM_SCGC2_DAC0_MASK;
	SIM->SCGC2 |= SIM_SCGC2_DAC1_MASK;

	DAC0->C0 = DAC_C0_DACEN_MASK | DAC_C0_DACRFS_MASK | DAC_C0_DACTRGSEL_MASK;
	DAC1->C0 = DAC_C0_DACEN_MASK | DAC_C0_DACRFS_MASK | DAC_C0_DACTRGSEL_MASK;
}

/**
 * @brief Escribe un valor al DAC (canal 0).
 *
 * Divide el dato en bytes low y high y los carga en los registros DAT[0].DATL y DAT[0].DATH.
 *
 * @param dac Puntero al DAC (DAC0 o DAC1).
 * @param data Valor digital a convertir.
 */
void DAC_SetData (DAC_t dac, DACData_t data)
{
	dac->DAT[0].DATL = DAC_DATL_DATA0(data);
	dac->DAT[0].DATH = DAC_DATH_DATA1(data >> DAC_DATL_DATA0_WIDTH);
}
