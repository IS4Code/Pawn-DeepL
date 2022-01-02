#include "cache.h"

#include "main.h"
#include "json/json11.hpp"

#include <string>
#include <fstream>
#include <streambuf>
#include <unordered_map>

std::unordered_map<std::string, std::string> cache_data;

void cache::load()
{
	std::ifstream in("scriptfiles/deepl_cache.txt");
	cache_data.clear();
	std::string err;
	for(std::string key; std::getline(in, key); )
	{
		std::string value;
		if(!std::getline(in, value)) break;
		auto json_val = json11::Json::parse(value, err);
		if(json_val == json11::Json())
		{
			logprintf("JSON parsing error: %s", err.c_str());
			break;
		}
		cache_data[std::move(key)] = json_val.string_value();
	}
}

const std::string &cache::get(const std::string &key)
{
	return cache_data[key];
}

void cache::set_or_get(const std::string &key, std::string &value)
{
	auto &ref = cache_data[key];
	if(!ref.empty())
	{
		value = ref;
		return;
	}
	ref = value;
	std::ofstream out("scriptfiles/deepl_cache.txt");
	std::string buffer;
	for(const auto &pair : cache_data)
	{
		buffer.clear();
		json11::Json(pair.second).dump(buffer);
		out << pair.first << std::endl << buffer << std::endl;
	}
	out.close();
}
