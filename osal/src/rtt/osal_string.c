#include <rtthread.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include "osal.h"

int osal_memncmp(const void *buf1, const void *buf2, unsigned long size)
{
	return memcmp(buf1, buf2, size);
}
// EXPORT_SYMBOL(osal_memncmp);
int osal_strcmp(const char *s1, const char *s2)
{
	return strcmp(s1, s2);
}
// EXPORT_SYMBOL(osal_strcmp);
int osal_strncmp(const char *s1, const char *s2, unsigned long size)
{
	return strncmp(s1, s2, size);
}
// EXPORT_SYMBOL(osal_strncmp);
int osal_strcasecmp(const char *s1, const char *s2)
{
	while (*s1 && *s2) {
		int c1 = tolower(*s1);
		int c2 = tolower(*s2);
		if (c1 != c2) {
			return c1 - c2;
		}
		s1++;
		s2++;
	}
	return *s1 - *s2;
}
// EXPORT_SYMBOL(osal_strcasecmp);
int osal_strncasecmp(const char *s1, const char *s2, unsigned long size)
{
	while (size && *s1 && *s2) {
		int c1 = tolower(*s1);
		int c2 = tolower(*s2);
		if (c1 != c2) {
			return c1 - c2;
		}
		s1++;
		s2++;
		size--;
	}
	if (size == 0) {
		return 0;
	}
	return *s1 - *s2;
}
// EXPORT_SYMBOL(osal_strncasecmp);
char *osal_strchr(const char *s, int n)
{
	return strchr(s, n);
}
// EXPORT_SYMBOL(osal_strchr);
char *osal_strnchr(const char *s, int count, int c)
{
	while (count-- > 0) {
		if (*s == c) {
			return (char *)s;
		}
		s++;
	}
	return NULL;
}
// EXPORT_SYMBOL(osal_strnchr);
char *osal_strrchr(const char *s, int c)
{
	return strrchr(s, c);
}
// EXPORT_SYMBOL(osal_strrchr);
char *osal_strstr(const char *s1, const char *s2)
{
	return strstr(s1, s2);
}
// EXPORT_SYMBOL(osal_strstr);
char *osal_strnstr(const char *s1, const char *s2, int len)
{
	int l2 = osal_strnlen(s2, len);
	if (!l2) {
		return (char *)s1;
	}
	int l1 = len;
	while (l1 >= l2) {
		l1--;
		if (!osal_strncmp(s1, s2, l2)) {
			return (char *)s1;
		}
		s1++;
	}
	return NULL;
}
// EXPORT_SYMBOL(osal_strnstr);
unsigned int osal_strlen(const char *s)
{
	return strlen(s);
}
// EXPORT_SYMBOL(osal_strlen);
unsigned int osal_strnlen(const char *s, unsigned int size)
{
	const char *sc;
	for (sc = s; size-- && *sc != '\0'; ++sc);
	return sc - s;
}
// EXPORT_SYMBOL(osal_strnlen);
char *osal_strpbrk(const char *s1, const char *s2)
{
	return strpbrk(s1, s2);
}
// EXPORT_SYMBOL(osal_strpbrk);
char *osal_strsep(char **s, const char *ct)
{
	char *sbegin = *s;
	char *end;

	if (sbegin == NULL) {
		return NULL;
	}

	end = osal_strnchr(sbegin, osal_strnlen(sbegin, INT_MAX), *ct);
	if (end) {
		*end++ = '\0';
		*s = end;
	} else {
		*s = NULL;
	}
	return sbegin;
}
// EXPORT_SYMBOL(osal_strsep);
unsigned int osal_strspn(const char *s, const char *accept)
{
	return strspn(s, accept);
}
// EXPORT_SYMBOL(osal_strspn);
unsigned int osal_strcspn(const char *s, const char *reject)
{
	return strcspn(s, reject);
}
// EXPORT_SYMBOL(osal_strcspn);
void *osal_memscan(void *addr, int c, int size)
{
	unsigned char *p = (unsigned char *)addr;
	while (size--) {
		if (*p == (unsigned char)c) {
			return p;
		}
		p++;
	}
	return NULL;
}
// EXPORT_SYMBOL(osal_memscan);
int osal_memcmp(const void *cs, const void *ct, int count)
{
	return memcmp(cs, ct, count);
}
// EXPORT_SYMBOL(osal_memcmp);
void *osal_memchr(const void *s, int c, int n)
{
	return memchr(s, c, n);
}
// EXPORT_SYMBOL(osal_memchr);
void *osal_memchr_inv(const void *start, int c, int bytes)
{
	const unsigned char *p = (const unsigned char *)start;
	while (bytes--) {
		if (*p != (unsigned char)c) {
			return (void *)p;
		}
		p++;
	}
	return NULL;
}
// EXPORT_SYMBOL(osal_memchr_inv);
unsigned long long osal_strtoull(const char *cp, char **endp, unsigned int base)
{
	unsigned long long result = 0;
	int negative = 0;

	if (*cp == '-') {
		negative = 1;
		cp++;
	}

	while (*cp) {
		int digit;
		if (*cp >= '0' && *cp <= '9') {
			digit = *cp - '0';
		} else if (*cp >= 'a' && *cp <= 'f') {
			digit = *cp - 'a' + 10;
		} else if (*cp >= 'A' && *cp <= 'F') {
			digit = *cp - 'A' + 10;
		} else {
			break;
		}
		if (digit >= base) {
			break;
		}
		result = result * base + digit;
		cp++;
	}

	if (endp) {
		*endp = (char *)cp;
	}

	return negative ? -result : result;
}
// EXPORT_SYMBOL(osal_strtoull);
unsigned long osal_strtoul(const char *cp, char **endp, unsigned int base)
{
	return (unsigned long)osal_strtoull(cp, endp, base);
}
// EXPORT_SYMBOL(osal_strtoul);
long osal_strtol(const char *cp, char **endp, unsigned int base)
{
	return (long)osal_strtoull(cp, endp, base);
}
// EXPORT_SYMBOL(osal_strtol);
long long osal_strtoll(const char *cp, char **endp, unsigned int base)
{
	return (long long)osal_strtoull(cp, endp, base);
}
// EXPORT_SYMBOL(osal_strtoll);

char *osal_strcpy(char *dest, const char *src)
{
	return strcpy(dest, src);
}
// EXPORT_SYMBOL(osal_strcpy);

char *osal_strncpy(char *dest, const char *src, unsigned long size)
{
	return strncpy(dest, src, size);
}
// EXPORT_SYMBOL(osal_strncpy);

