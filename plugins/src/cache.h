#ifndef CACHE_H_INCLUDED
#define CACHE_H_INCLUDED

#include <string>

namespace cache
{
	void load();
	const std::string &get(const std::string &key);
	void set(const std::string &key, const std::string &value);
}

#endif
