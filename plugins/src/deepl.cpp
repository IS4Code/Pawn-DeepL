#include "deepl.h"
#include "main.h"
#include "cache.h"

#define CURL_STATICLIB
#include <curl/curl.h>

#include <memory>
#include <queue>
#include <unordered_map>
#include <vector>

std::string deepl::auth_header;
std::string deepl::endpoint_url = "https://api-free.deepl.com/v2/translate";

struct curlm_deleter
{
	void operator()(CURLM *curlm)
	{
		curl_multi_cleanup(curlm);
	}
};

struct curl_deleter
{
	void operator()(CURL *curl)
	{
		curl_easy_cleanup(curl);
	}
};

struct curl_slist_deleter
{
	void operator()(curl_slist *slist)
	{
		curl_slist_free_all(slist);
	}
};

typedef std::unique_ptr<CURL, curl_deleter> curl_handle;

std::unique_ptr<CURLM, curlm_deleter> curlm_ptr;

std::queue<curl_handle> curl_pool;

std::unique_ptr<curl_slist, curl_slist_deleter> http_headers = []()
{
	curl_slist *headers = nullptr;
	headers = curl_slist_append(headers, "Accept: application/json");
	return std::unique_ptr<curl_slist, curl_slist_deleter>(headers);
}();

void deepl::load()
{
	curlm_ptr = std::unique_ptr<CURLM, curlm_deleter>(curl_multi_init());
}

void deepl::unload()
{
	curl_pool.empty();
	curlm_ptr = nullptr;
}

curl_handle curl_easy_get()
{
	curl_handle handle;
	if(!curl_pool.empty())
	{
		std::swap(handle, curl_pool.front());
		curl_pool.pop();
	}else{
		handle = curl_handle(curl_easy_init());
	}
	return handle;
}

class curl_request
{
	curl_handle handle;
	std::string fields;
	deepl::callback_func callback;

	std::string response;

	size_t write(char *ptr, size_t size, size_t nmemb)
	{
		response.append(ptr, size * nmemb);
		return size * nmemb;
	}

public:
	curl_request(curl_handle handle, std::string fields, deepl::callback_func callback) : handle(std::move(handle)), fields(std::move(fields)), callback(std::move(callback))
	{
		auto curl = this->handle.get();
		curl_slist *headers = nullptr;
		headers = curl_slist_append(headers, deepl::auth_header.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, this->fields.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, this->fields.size());

		curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +[](void *ptr, size_t size, size_t nmemb, void *data) -> size_t
		{
			return reinterpret_cast<curl_request*>(data)->write(reinterpret_cast<char*>(ptr), size, nmemb);
		});
	}

	curl_handle finish(long status_code)
	{
		bool should_cache = callback(response, status_code);
		if(should_cache && status_code == 200)
		{
			cache::set_or_get(fields, response);
		}
		return std::move(handle);
	}
};

std::unordered_map<CURL*, std::unique_ptr<curl_request>> requests;

class fake_request
{
	const std::string &response;
	deepl::callback_func callback;

public:
	fake_request(const std::string &response, deepl::callback_func callback) : response(std::move(response)), callback(std::move(callback))
	{

	}

	void process() const
	{
		callback(response, 200);
	}
};

std::vector<fake_request> fake_requests;

void deepl::process()
{
	auto multi = curlm_ptr.get();

	int running;
	curl_multi_perform(multi, &running);

	if(static_cast<unsigned int>(running) < requests.size())
	{
		CURLMsg *msg;

		do{
			int msgq = 0;
			msg = curl_multi_info_read(multi, &msgq);
			if(msg && (msg->msg == CURLMSG_DONE))
			{
				CURL *curl = msg->easy_handle;
				curl_multi_remove_handle(multi, curl);
				
				auto it = requests.find(curl);
				if(it == requests.end())
				{
					curl_easy_cleanup(curl);
				}else{
					long http_code = 0;
					curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

					std::unique_ptr<curl_request> request;
					std::swap(request, it->second);
					requests.erase(it);

					curl_pool.push(request->finish(http_code));
				}
			}
		}while(msg);
	}

	decltype(fake_requests) copy;
	std::swap(copy, fake_requests);
	for(const auto &req : copy)
	{
		req.process();
	}
}

int deepl::make_request(bool preserve_formatting, const char *tag_handling, const char *formality, const char *split_sentences, const char *source_lang, const char *target_lang, const std::string&text, deepl::callback_func callback)
{
	auto curl_handle = curl_easy_get();
	auto curl = curl_handle.get();

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, http_headers.get());

	curl_easy_setopt(curl, CURLOPT_URL, endpoint_url.c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	
	std::string fields;

	fields += "text=";
	char *encoded = curl_easy_escape(curl, text.c_str(), text.size());
	fields += encoded;
	curl_free(encoded);

	if(preserve_formatting)
	{
		fields += "&preserve_formatting=1";
	}

	if(tag_handling)
	{
		fields += "&tag_handling=";
		encoded = curl_easy_escape(curl, tag_handling, 0);
		fields += encoded;
		curl_free(encoded);
	}

	if(formality)
	{
		fields += "&formality=";
		encoded = curl_easy_escape(curl, formality, 0);
		fields += encoded;
		curl_free(encoded);
	}

	if(split_sentences)
	{
		fields += "&split_sentences=";
		encoded = curl_easy_escape(curl, split_sentences, 0);
		fields += encoded;
		curl_free(encoded);
	}

	if(source_lang)
	{
		fields += "&source_lang=";
		encoded = curl_easy_escape(curl, source_lang, 0);
		fields += encoded;
		curl_free(encoded);
	}

	if(target_lang)
	{
		fields += "&target_lang=";
		encoded = curl_easy_escape(curl, target_lang, 0);
		fields += encoded;
		curl_free(encoded);
	}

	const auto &cached = cache::get(fields);
	if(!cached.empty())
	{
		fake_requests.push_back(fake_request(cached, std::move(callback)));
		return 0;
	}

	requests[curl] = std::unique_ptr<curl_request>(new curl_request(std::move(curl_handle), std::move(fields), std::move(callback)));
	return curl_multi_add_handle(curlm_ptr.get(), curl);
}

