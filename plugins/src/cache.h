#ifndef CACHE_H_INCLUDED
#define CACHE_H_INCLUDED

#include <string>

namespace cache
{
	void load();
	const std::string &get(const std::string &key);
	void set_or_get(const std::string &key, std::string &value);
}

#endif
