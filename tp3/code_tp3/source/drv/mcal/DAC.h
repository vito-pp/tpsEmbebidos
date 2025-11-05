
#ifndef DAC_H_
#define DAC_H_

#include "hardware.h"
#include "MK64F12.h"

typedef DAC_Type *DAC_t;
typedef uint16_t DACData_t;


/**
 * @brief   Inicializa DAC0 y DAC1.
 * @details Habilita clocks (SIM_SCGC2) y configura C0:
 *          - DACEN: habilita el convertidor
 *          - DACRFS: referencia VDDA
 *          - DACTRGSEL: disparo por software
 */
void DAC_Init	 (void);
/**
 * @brief   Escribe un valor al DAC (canal 0).
 * @param   dac   Puntero al DAC (DAC0/DAC1).
 * @param   data  Valor digital a convertir (ancho según resolución del DAC).
 * @note    Carga DATL/DATH con el dato (partido en low/high).
 */
void DAC_SetData (DAC_t, DACData_t);

void DAC_PISR (void);

#endif /* void DAC_H_ */
