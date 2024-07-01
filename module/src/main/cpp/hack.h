//
// Created by Perfare on 2020/7/4.
//

#ifndef ZYGISK_IL2CPPDUMPER_HACK_H
#define ZYGISK_IL2CPPDUMPER_HACK_H

#include <stddef.h>
#include <dobby.h>
#include <sys/mman.h>

void hack_prepare(const char *game_data_dir, void *data, size_t length);

#define HOOK_DEF(ret, func, ...) \
  ret (*orig_##func)(__VA_ARGS__); \
  ret new_##func(__VA_ARGS__)


#define HOOK(FN, NEW, ORIG) if ((void *)FN) {DobbyHook((void *)FN, (void *) new_lua_pcall, (void **) &ORIG);}

#define UNHOOK(FN) DobbyDestroy((void *)FN)


#endif //ZYGISK_IL2CPPDUMPER_HACK_H
