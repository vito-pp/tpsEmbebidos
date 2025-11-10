/**
 * @file bitstream.h
 * @brief Archivo de cabecera para el manejo de bitstreams en comunicación UART.
 *
 * Este archivo define funciones para formatear y deformatear datos en frames UART,
 * incluyendo bits de start, paridad y stop. Soporta cálculos de paridad par y conversiones
 * entre bytes de datos y arreglos de bits booleanos.
 */

#ifndef BITSTREAM
#define BITSTREAM

#include <stdint.h>
#include <stdbool.h>

/**
 * @def BITSTREAM_SIZE
 * @brief Tamaño del bitstream en bits (1 start + 8 data + 1 parity + 1 stop).
 */
#define BITSTREAM_SIZE 11u // 1 start + 8 data + 1 parity + 1 stop

/**
 * @brief Formatea un byte de datos en un frame UART de 11 bits.
 *
 * Agrega bit de start (0), los 8 bits de datos (LSB-first), bit de paridad y bit de stop (1).
 *
 * @param data El byte de datos a formatear.
 * @return El frame UART como uint16_t (solo los 11 bits inferiores son relevantes).
 */
uint16_t data_to_uart(uint8_t data);

/**
 * @brief Convierte un frame UART de 11 bits de vuelta a un byte de datos.
 *
 * Extrae los 8 bits de datos del frame, ignorando start, paridad y stop.
 *
 * @param frame El frame UART como uint16_t.
 * @return El byte de datos extraído.
 */
uint8_t uart_to_data(uint16_t frame);

/**
 * @brief Calcula el bit de paridad par para un byte de datos.
 *
 * Cuenta la cantidad de bits en 1; si es par, retorna true (1), sino false (0).
 *
 * @param data El byte de datos para calcular la paridad.
 * @return true si la paridad es par (bit de paridad = 1), false en caso contrario.
 */
bool parity_bit(const uint8_t data);

/**
 * @brief Formatea un byte de datos en un arreglo de 11 bits booleanos.
 *
 * Utiliza data_to_uart para generar el frame y lo descompone en un arreglo bool[11].
 * Modifica el arreglo out proporcionado.
 *
 * @param data El byte de datos a formatear.
 * @param out Arreglo de 11 booleanos donde se almacenará el bitstream.
 */
void format_bitstream(uint8_t data, bool out[11]);

/**
 * @brief Deformatea un arreglo de 11 bits booleanos en un byte de datos.
 *
 * Extrae los 8 bits de datos (posiciones 1 a 8) y los convierte en un char (uint8_t).
 *
 * @param in Arreglo de 11 booleanos que representa el bitstream.
 * @return El byte de datos extraído como char.
 */
char deformat_bitstream(bool in[11]);

#endif // BITSTREAM