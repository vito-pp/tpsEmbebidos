#include "credentials.h"

Credentials_t credentials[] =
{
    /* Floor 1: 4 positions */
    {.id = 64199420, .pin = 1234,  .floor = 1, .present = false},
    {.id = 12345678, .pin = 00001, .floor = 1, .present = false},
    {.id = 11110001, .pin = 1001,  .floor = 1, .present = false},
    {.id = 11110002, .pin = 1002,  .floor = 1, .present = false},

    /* Floor 2: 4 positions */
    {.id = 11111111, .pin = 00002, .floor = 2, .present = false},
    {.id = 22220001, .pin = 2001,  .floor = 2, .present = false},
    {.id = 22220002, .pin = 2002,  .floor = 2, .present = false},
    {.id = 22220003, .pin = 2003,  .floor = 2, .present = false},

    /* Floor 3: 4 positions */
    {.id = 33330001, .pin = 3001,  .floor = 3, .present = false},
    {.id = 33330002, .pin = 3002,  .floor = 3, .present = false},
    {.id = 33330003, .pin = 3003,  .floor = 3, .present = false},
    {.id = 33330004, .pin = 3004,  .floor = 3, .present = false}
};

const size_t number_of_users = sizeof(credentials) / sizeof(credentials[0]);
