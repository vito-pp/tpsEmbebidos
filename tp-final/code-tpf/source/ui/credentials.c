#include "credentials.h"

Credentials_t credentials[] =
{
    {.id = 64199420,.pin = 1234, .floor = 1},
    {.id = 12345678,.pin = 00001, .floor = 1},
    {.id = 11111111,.pin = 00002, .floor = 2}
};

const size_t number_of_users = sizeof(credentials) / sizeof(credentials[0]);
