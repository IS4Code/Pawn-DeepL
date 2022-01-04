#ifndef DEEPL_H_INCLUDED
#define DEEPL_H_INCLUDED

#include <string>
#include <functional>

namespace deepl
{
	typedef std::function<void(const std::string &response, long status_code)> callback_func;

	void load();
	void process();
	void unload();

	int make_request(bool preserve_formatting, const char *tag_handling, const char *source_lang, const char *target_lang, const std::string &text, callback_func callback);

	extern std::string auth_key;
}

#endif
