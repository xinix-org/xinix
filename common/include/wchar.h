#pragma once

#ifndef __cplusplus
#ifdef __WCHAR_TYPE__
typedef __WCHAR_TYPE__ wchar_t;
#else
typedef int wchar_t;
#endif
#endif

static_assert(sizeof(wchar_t)==4);

#ifdef __WINT_TYPE__
typedef __WINT_TYPE__ wint_t;
#else
typedef int wint_t;
#endif