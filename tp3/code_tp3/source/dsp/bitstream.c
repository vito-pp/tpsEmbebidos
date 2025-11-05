#include "bitstream.h"

// Formatea un byte de datos en un bitstream con start, parity y stop bits.
uint16_t data_to_uart(uint8_t data){
    // use existing parity_bit(data) function (returns bool)
    bool p = parity_bit(data);
    // pack: bit0 = start(0), bits1..8 = data (LSB-first), bit9 = parity, bit10 = stop(1)
    uint16_t frame = ((uint16_t)1 << 10)         // stop bit at bit10
                   | ((uint16_t)p << 9)          // parity at bit9
                   | ((uint16_t)data << 1);      // data in bits1..8
                   // Start en cero implicito
    return frame;
}

// Calcula el bit de paridad par para un byte de datos entrante.
bool parity_bit(const uint8_t data){
    int counter = 0;
    // Procesar el byte de datos entrante
    for (int i = 0; i < 8; i++) {
        bool bit = (data >> (7 - i)) & 0x01;
        if (bit){
            counter ++;
        }
    }
    if (counter % 2 == 0){
        return true; //Hay una cantidad par de unos. P = 1.
    } else {
        return false; //Hay una cantidad impar de unos. P = 0.
    }
}

//--------------FUNCION DE ENTRADA A NCO-----------------
//-------------------------------------------------------
// Funcion para modificar a formato bool[11]. 
// Modifica el arreglo original introducido en la funcion.
void format_bitstream(uint8_t data, bool out[11]){
    uint16_t frame = data_to_uart(data);
    for (int i = 0; i < 11; ++i){
        out[i] = (frame >> i) & 1;
    }
}

uint8_t deformat_bitstream(bool in[11]){
    uint8_t frame = 0;
    for (int i = 1; i < 9; ++i){
        if (in[i]){
            frame |= (1 << i);
        }
    }
    return frame;
}

// Funcion que deforma un arreglo de 11 bits booleanos en un byte de datos.
// Devuelve dicho byte de datos para ser enviado al UART.
char deformat_bitstream(bool in[11]){
    char result = 0;
    // Leer bits de datos (descartar start, parity y stop)
    for (int i = 0; i < 8; ++i){
        if (in[i + 1]){ // Descartar start bit
            result |= (1 << i);
        }
    }
    return result;
}