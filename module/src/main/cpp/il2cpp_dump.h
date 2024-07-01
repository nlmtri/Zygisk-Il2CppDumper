//
// Created by Perfare on 2020/7/4.
//

#ifndef ZYGISK_IL2CPPDUMPER_IL2CPP_DUMP_H
#define ZYGISK_IL2CPPDUMPER_IL2CPP_DUMP_H

#include <stdint.h>

void il2cpp_api_init(void *handle);

void il2cpp_dump(const char *outDir);

void il2cpp_start_world();

void il2cpp_stop_world();

void* il2cpp_get_fun_addr(const char* imageName, const char* nameSpace, const char* className, const char* methodName, int argCount);

uint64_t get_il2pp_base();

#endif //ZYGISK_IL2CPPDUMPER_IL2CPP_DUMP_H
