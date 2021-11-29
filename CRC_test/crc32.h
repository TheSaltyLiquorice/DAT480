#pragma once

#include <inttypes.h>
void generate_table(uint32_t(&table)[256]);
uint32_t crc_cal(uint32_t* table, const char* buf, size_t len);

