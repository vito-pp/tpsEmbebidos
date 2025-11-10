/**
 * @file fir_coefs.h
 * @brief Coeficientes del filtro FIR generados por herramienta de diseño.
 *
 * Este archivo define coeficientes para un filtro FIR de paso bajo, estable y de fase lineal.
 * Generado por MATLAB y Signal Processing Toolbox.
 */

#ifndef _FIR_COEFS_
#define _FIR_COEFS_

/*
 * Discrete-Time FIR Filter (real)
 * -------------------------------
 * Filter Structure  : Direct-Form FIR
 * Filter Length     : 26
 * Stable            : Yes
 * Linear Phase      : Yes (Type 1)
 */

/**
 * @def FS_ADC
 * @brief Frecuencia de muestreo del ADC (12000 Hz).
 */
#define FS_ADC 12000

/**
 * @def N
 * @brief Longitud del filtro FIR (26).
 */
#define N 26

/**
 * @var h
 * @brief Arreglo de coeficientes del filtro FIR (float).
 */
const float h[N] =
{
  0.0009219922358,  0.00335702789, 0.006668464281, 0.007552863564,  0.00103278039,
   -0.01547866594, -0.03695775568, -0.04899393767, -0.03326134011,  0.02089643851,
     0.1057718769,    0.194080919,   0.2505506277,   0.2505506277,    0.194080919,
     0.1057718769,  0.02089643851, -0.03326134011, -0.04899393767, -0.03695775568,
   -0.01547866594,  0.00103278039, 0.007552863564, 0.006668464281,  0.00335702789,
  0.0009219922358
};

#endif // _FIR_COEFS_