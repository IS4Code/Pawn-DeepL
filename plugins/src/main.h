#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include "sdk/amx/amx.h"
#include <unordered_map>
#include <memory>

typedef void(*logprintf_t)(const char* format, ...);
extern logprintf_t logprintf;

std::weak_ptr<AMX> get_amx(AMX *amx);

#endif
