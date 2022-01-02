#include "cache.h"

#include "json/json11.hpp"

#include <string>
#include <fstream>
#include <streambuf>

json11::Json cache_data;

void cache::load()
{
	std::ifstream in("scriptfiles/deepl_cache.json");
	std::string str((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	std::string err;
	cache_data = json11::Json::parse(str, err);
}

const std::string &cache::get(const std::string &key)
{
	return cache_data[key].string_value();
}

void cache::set(const std::string &key, const std::string &value)
{
	auto data = cache_data.object_items();
	data[key] = value;
	cache_data = json11::Json(data);
	std::string buffer;
	cache_data.dump(buffer);
	std::ofstream out("scriptfiles/deepl_cache.json");
	out << buffer;
	out.close();
}
