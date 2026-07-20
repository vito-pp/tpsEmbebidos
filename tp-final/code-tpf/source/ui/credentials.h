#ifndef _CREDENTIALS_H_
#define _CREDENTIALS_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct Credentials_t
{
    uint32_t id;
    uint32_t pin;
    uint8_t floor; // 1, 2 or 3
    bool present;
} Credentials_t;

extern Credentials_t credentials[];
extern const size_t number_of_users;

#endif // _CREDENTIALS_H_
