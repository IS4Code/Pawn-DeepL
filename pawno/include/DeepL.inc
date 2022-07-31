/** DeepL v1.0 by IllidanS4 **/
//github.com/IllidanS4/Pawn-DeepL

#if defined _inc_DeepL
#undef _inc_DeepL
#endif
#if defined _DeepL_included
#endinput
#endif
#define _DeepL_included

forward DeepL_OnTranslationDone(bool:success, result[], from[], cookie);

native DeepL_SetAuthKey(const key[]);
native DeepL_Translate(const text[], const from[], const to[], const callback[] = #DeepL_OnTranslationDone, bool:preserve_formatting = true, const tag_handling[] = "", cookie = 0);
native DeepL_LoadCache();
