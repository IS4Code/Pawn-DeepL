DeepL Plugin v1.3
==========

This plugin adds integration for the [DeepL Translate API](https://developers.deepl.com/docs/api-reference/translate). To use the plugin, you have to provide your [DeepL authentication key](https://developers.deepl.com/docs/getting-started/auth#authentication).

*There are no warranties nor support for the quality of the translation, which is handled purely by DeepL and system-defined locale conversions.*

## Usage

Before requesting translation, call `DeepL_SetAuthKey` to set your authentication key:
```pawn
DeepL_SetAuthKey("cbae67e8-63cd-4f7f-a0c9-a2a32905679d:fx"); // example key
```

To request translation via the API, call `DeepL_Translate`:
```pawn
native DeepL_Translate(
  const text[], // the text to translate
  const from[], // the source language and encoding
  const to[], // the target language and encoding
  const callback[] = #DeepL_OnTranslationDone, // the callback to invoke when translation is ready
  cookie = 0, // the cookie to pass to callback
  bool:preserve_formatting = true, // see the DeepL API for description of these parameters
  const tag_handling[] = "",
  const formality[] = "",
  const split_sentences[] = ""
);
```

The function returns `0` on success.

The callback function is in this form:
```pawn
forward bool:DeepL_OnTranslationDone(
  bool:success, // whether translation was produced or not
  result[], // if success, the translated text, otherwise the error message
  from[], // the detected source language
  cookie // cookie passed to DeepL_Translate
);
```

If the returned value is `true` and the text was successfully translated, the result is cached.

### Specifying language

The value of `from` and `to` is in the form <code>*language*:*locales*</code>, where <code>*locales*</code> is a `|`-separated list of locale names (see [`std::locale`](https://en.cppreference.com/w/cpp/locale/locale/locale) for more description). If the first locale name is not found, it tries the second one, and so forth.

On POSIX systems, you may use `locale -a` to list the existing locale names. On Unix, it is possible to define a custom locale using the [`localedef` command](https://man7.org/linux/man-pages/man1/localedef.1.html). For example, to define the locale `en_US.CP1250`, you could use `localedef -i en_US -f CP1250 'en_US.CP1250'` which you can then use as the argument in the form `"en:en_US.CP1250|.1250"`. On Windows, <code>.*codepage*</code> can be used directly for a locale based on a particular codepage.

The locales may be omitted altogether, in which case UTF-8 will be used for `text`/`result`.

### Caching

Successful responses from DeepL are cached in the file `scriptfiles/deepl_cache.txt`. This file is composed of alternating key-value lines, where the key is formed from the API parameters, and value is the encoded JSON response.

The cache file is written after every translation, when `true` is returned from the callback. It may also be modified freely to change the particular translations. If the file is modified on-the-fly while the server is running, call `DeepL_LoadCache` to reload it. 

## Installation
Download the latest [release](//github.com/IS4Code/Pawn-DeepL/releases/latest) for your platform to the "plugins" directory and add "DeepL" (or "DeepL.so" on Linux) to the `plugins` line in server.cfg.

Include [DeepL.inc](pawno/include/DeepL.inc) in your Pawn program and you are done.

## Building
Use Visual Studio to build the project on Windows, or `make` or `make static` on Linux. Requires GCC >= 4.9 and [cURL](https://curl.se/docs/install.html) to be installed.
