#include "credentials.h"

const Credentials_t credentials[] =
{
    {.id = 64199420,.pin = 1234},
    {.id = 12345678,.pin = 00001}
};

const size_t number_of_users = sizeof(credentials) / sizeof(credentials[0]);