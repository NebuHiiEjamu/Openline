/* (c)2003 Hotsprings Inc. Licensed under GPL - see LICENSE in HotlineSources diresctory */

#pragma once
#include "stdarg.h"

#ifndef NULL
#define NULL	0L
#endif

#if WIN32
typedef unsigned int size_t;
#else
typedef unsigned long size_t;
#endif


#ifndef OFFSET_OF
#define OFFSET_OF(T, member)		((UInt32)&(((T*)0)->member))
#endif


extern "C" {

void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);

void *memcpy(void *dst, const void *src, size_t len);
void *memmove(void *dst, const void *src, size_t len);
void *memset(void *dst, int val, size_t n);
void *memchr(const void *src, int val, size_t n);
void *memrchr(const void *src, int val, size_t n);
int memcmp(const void *src1, const void *src2, size_t n);

size_t strlen(const char *str);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t len);
char *strcat(char *dst, const char *src);
char *strncat(char *dst, const char *src, size_t len);
int strcmp(const char *str1, const char *str2);
int strncmp(const char *str1, const char *str2, size_t len);
char *strchr(const char *str, int chr);
char *strrchr(const char *str, int chr);
char *strpbrk(const char *str, const char *set);
size_t strspn(const char *str, const char *set);
size_t strcspn(const char *str, const char *set);
char *strtok(char *str, const char *set);
char *strstr(const char *str, const char *pat);
char *strrev(char *str);

unsigned char *pstrcpy(unsigned char *dst, const unsigned char *src);
unsigned char *pstrncpy(unsigned char *dst, const unsigned char *src, size_t len);
unsigned char *pstrcat(unsigned char *dst, const unsigned char *src);
unsigned char *pstrncat(unsigned char *dst, const unsigned char *src, size_t len);
int pstrcmp(const unsigned char *str1, const unsigned char *str2);
int pstrncmp(const unsigned char *str1, const unsigned char *str2, size_t len);
unsigned char *pstrchr(const unsigned char *str, unsigned char chr);
unsigned char *pstrrchr(const unsigned char *str, unsigned char chr);
size_t pstrspn(const unsigned char *str, const unsigned char *set);
size_t pstrcspn(const unsigned char *str, const unsigned char *set);

long strtol(const char *str, char **end, int base);
unsigned long strtoul(const char *str, char **end, int base);
char *ltostr(long val, char *str, int base);
char *ultostr(unsigned long val, char *str, int base);

int sprintf(char *str, const char *fmt, ...);
int vsprintf(char *str, const char *fmt, ::va_list arg);

void exit(int status);
int atexit(void (*func)(void));
void abort(void);

typedef int (*_compare_func)(const void *a, const void *b);
void *bsearch(const void *key, const void *base, size_t nmemb, size_t size, _compare_func compare);
void qsort(void *base, size_t nmemb, size_t size, _compare_func compare);

extern int errno;

/*
extern unsigned char __ctype_map[];
extern unsigned char __lower_map[];
extern unsigned char __upper_map[];
*/
}	// extern "C"

inline size_t pstrlen(const unsigned char *str)
{
	return *str;	// str[0]
}

inline const unsigned char *pstrtext(const unsigned char *str)
{
	return str+1;
}

inline unsigned char *pstrtext(unsigned char *str)
{
	return str+1;
}
/*
#ifndef __cctype__

#define __control_char		0x01
#define __motion_char		0x02
#define __space_char		0x04
#define __punctuation		0x08
#define __digit				0x10
#define __hex_digit			0x20
#define __lower_case		0x40
#define __upper_case		0x80

#define __letter			(__lower_case | __upper_case)
#define __alphanumeric		(__letter | __digit)
#define __graphic			(__alphanumeric | __punctuation)
#define __printable			(__graphic | __space_char)
#define __whitespace		(__motion_char | __space_char)
#define __control			(__motion_char | __control_char)

inline int isalnum(int c)	{	return (__ctype_map[(unsigned char)c] & __alphanumeric) != 0;	}
inline int isalpha(int c)	{	return (__ctype_map[(unsigned char)c] & __letter) != 0;			}
inline int iscntrl(int c)	{	return (__ctype_map[(unsigned char)c] & __control) != 0;		}
inline int isdigit(int c)	{	return (__ctype_map[(unsigned char)c] & __digit) != 0;			}
inline int isgraph(int c)	{	return (__ctype_map[(unsigned char)c] & __graphic) != 0;		}
inline int islower(int c)	{	return (__ctype_map[(unsigned char)c] & __lower_case) != 0;		}
inline int isprint(int c)	{	return (__ctype_map[(unsigned char)c] & __printable) != 0;		}
inline int ispunct(int c)	{	return (__ctype_map[(unsigned char)c] & __punctuation) != 0;	}
inline int isspace(int c)	{	return (__ctype_map[(unsigned char)c] & __whitespace) != 0;		}
inline int isupper(int c)	{	return (__ctype_map[(unsigned char)c] & __upper_case) != 0;		}
inline int isxdigit(int c)	{	return (__ctype_map[(unsigned char)c] & __hex_digit) != 0;		}
inline int tolower(int c)	{	return __lower_map[(unsigned char)c];							}
inline int toupper(int c)	{	return __upper_map[(unsigned char)c];							}

#endif
*/