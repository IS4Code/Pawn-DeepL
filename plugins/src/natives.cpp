#include "natives.h"
#include "deepl.h"
#include "cache.h"
#include "main.h"

#include "json/json11.hpp"

#include <cstring>
#include <codecvt>
#include <locale>

char *split_locale(char *str)
{
	auto pos = std::strstr(str, ":");
	if(pos)
	{
		*pos = '\0';
		++pos;
	}
	return pos;
}

char *split_locale_alt(char *str)
{
	auto pos = std::strstr(str, "|");
	if(pos)
	{
		*pos = '\0';
		++pos;
	}
	return pos;
}

std::locale find_locale(char *&spec)
{
	char *nextspec;
	while(nextspec = split_locale_alt(spec))
	{
		try{
			return std::locale(spec);
		}catch(const std::runtime_error &)
		{

		}
		spec = nextspec;
	}
	return std::locale(spec);
}

static std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8;

namespace Natives
{
	cell SetAuthKey(AMX *amx, cell *params)
	{
		cell *key;
		amx_GetAddr(amx, params[1], &key);
		int len;
		amx_StrLen(key, &len);

		auto &str = deepl::auth_key = std::string(len, '\0');
		amx_GetString(&str[0], key, false, len + 1);
		return 1;
	}

	cell Translate(AMX *amx, cell *params)
	{
		cell *addr;
		amx_GetAddr(amx, params[1], &addr);
		int len;
		amx_StrLen(addr, &len);
		std::string text(len, '\0');
		amx_GetString(&text[0], addr, false, len + 1);

		char *from, *to, *tags;
		amx_StrParam(amx, params[2], from);
		amx_StrParam(amx, params[3], to);
		bool formatting = params[5];
		amx_StrParam(amx, params[6], tags);
		cell cookie = params[7];

		amx_GetAddr(amx, params[4], &addr);
		amx_StrLen(addr, &len);
		std::string callback(len, '\0');
		amx_GetString(&callback[0], addr, false, len + 1);

		if(auto from_locale = split_locale(from))
		{
			try{
				auto loc = find_locale(from_locale);
				std::wstring buffer(text.size(), L'\0');
				std::use_facet<std::ctype<wchar_t>>(loc).widen(text.data(), text.data() + text.size(), &buffer[0]);
				text = utf8.to_bytes(buffer.data());
			}catch(const std::exception &e)
			{
				logprintf("[DeepL] Translate error: %s (input locale %s)", e.what(), from_locale);
				return -1;
			}
		}

		std::locale *to_locale_ptr = nullptr;
		if(auto to_locale = split_locale(to))
		{
			try{
				to_locale_ptr = new std::locale(find_locale(to_locale));
			}catch(const std::exception &e)
			{
				logprintf("[DeepL] Translate error: %s (output locale %s)", e.what(), to_locale);
				return -1;
			}
		}

		auto amx_handle = get_amx(amx);
		return deepl::make_request(true, tags, from, to, text, [callback, amx_handle, cookie, to_locale_ptr](const std::string &result, long status_code)
		{
			if(auto handle = amx_handle.lock())
			{
				auto amx = handle.get();

				bool success = false;
				std::string message;
				std::string from;
				auto json = json11::Json::parse(result, message);
				if(json.is_object())
				{
					for(const auto &item : json["translations"].array_items())
					{
						if(item.is_object())
						{
							message = item["text"].string_value();
							from = item["detected_source_language"].string_value();
							success = true;
							break;
						}
					}
					if(!success)
					{
						const auto &msg = json["message"];
						message = msg.string_value();
					}
				}

				if(to_locale_ptr)
				{
					if(success)
					{
						try{
							auto buffer = utf8.from_bytes(message);
							message.resize(buffer.size(), '\0');
							std::use_facet<std::ctype<wchar_t>>(*to_locale_ptr).narrow(buffer.data(), buffer.data() + buffer.size(), '?', &message[0]);
						}catch(const std::exception &e)
						{
							message = e.what();
							success = false;
						}
					}
					delete to_locale_ptr;
				}

				int index;
				if(amx_FindPublic(amx, callback.c_str(), &index) != AMX_ERR_NONE)
				{
					return;
				}
				cell message_addr, *message_ptr;
				if(amx_Allot(amx, message.size() + 1, &message_addr, &message_ptr) != AMX_ERR_NONE)
				{
					return;
				}
				amx_SetString(message_ptr, message.c_str(), false, false, message.size() + 1);

				cell from_addr, *from_ptr;
				if(amx_Allot(amx, from.size() + 1, &from_addr, &from_ptr) != AMX_ERR_NONE)
				{
					amx_Release(amx, message_addr);
					return;
				}
				amx_SetString(from_ptr, from.c_str(), false, false, from.size() + 1);

				amx_Push(amx, cookie);
				amx_Push(amx, from_addr);
				amx_Push(amx, message_addr);
				amx_Push(amx, success);
				cell retval;
				amx_Exec(amx, &retval, index);

				amx_Release(amx, message_addr);
			}
		});
	}

	cell LoadCache(AMX *amx, cell *params)
	{
		cache::load();
		return 1;
	}
}

AMX_NATIVE_INFO natives[] = {
	{ "DeepL_SetAuthKey", Natives::SetAuthKey },
	{ "DeepL_Translate", Natives::Translate },
	{ "DeepL_LoadCache", Natives::LoadCache },
	{ nullptr, nullptr }
};
