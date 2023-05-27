#include "main.h"
#include "deepl.h"
#include "cache.h"
#include "natives.h"

#include "sdk/amx/amx.h"
#include "sdk/plugincommon.h"

logprintf_t logprintf;
extern void *pAMXFunctions;

std::unordered_map<AMX*, std::shared_ptr<AMX>> amx_map;

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports() 
{
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData)
{
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];

	deepl::load();
	cache::load();

	logprintf(" DeepL API v1.1 loaded");
	logprintf(" Created by IllidanS4");
	return true;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload()
{
	deepl::unload();

	logprintf(" DeepL API v1.0 unloaded");
}

PLUGIN_EXPORT void PLUGIN_CALL ProcessTick()
{
	deepl::process();
}

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx) 
{
	amx_Register(amx, natives, -1);
	return AMX_ERR_NONE;
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx) 
{
	amx_map.erase(amx);
	return AMX_ERR_NONE;
}

std::weak_ptr<AMX> get_amx(AMX *amx)
{
	auto &ref = amx_map[amx];
	if(ref) return ref;
	return ref = std::shared_ptr<AMX>(std::make_shared<bool>(false), amx);
}
