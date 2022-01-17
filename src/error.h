#pragma once
#include "ovbase.h"

bool error_init(void);
bool error_register_default_mapper(error_message_mapper generic_error_message_mapper);
void error_exit(void);
