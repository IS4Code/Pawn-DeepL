/** DeepL v1.2 by IS4 **/
//github.com/IS4Code/Pawn-DeepL

#if defined _inc_DeepL
#undef _inc_DeepL
#endif
#if defined _DeepL_included
#endinput
#endif
#define _DeepL_included

forward bool:DeepL_OnTranslationDone(bool:success, result[], from[], cookie);

native DeepL_SetAuthKey(const key[]);
native DeepL_SetEndpoint(const endpoint_url[]);
native DeepL_Translate(const text[], const from[], const to[], const callback[] = #DeepL_OnTranslationDone, cookie = 0, bool:preserve_formatting = true, const tag_handling[] = "", const formality[] = "", const split_sentences[] = "");
native DeepL_LoadCache();
