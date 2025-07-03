#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <cstdint>
#include <string>

int64_t iclamp64(int64_t i, int64_t min, int64_t max);


void set_startupcmd_values(std::string& shader_code, const char* name, const float values[4]);



#endif
