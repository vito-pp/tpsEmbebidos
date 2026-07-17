#ifndef _CREDENTIALS_H_
#define _CREDENTIALS_H_

#include <stdint.h>
#include <stddef.h>

typedef struct Credentials_t
{
    uint32_t id;
    uint32_t pin;
} Credentials_t;

extern const Credentials_t credentials[];
extern const size_t number_of_users;

#endif // _CREDENTIALS_H_