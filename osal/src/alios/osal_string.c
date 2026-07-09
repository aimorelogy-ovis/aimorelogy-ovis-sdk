#include <string.h>
#include "osal.h"

int osal_memncmp(const void *buf1, const void *buf2, unsigned long size)
{
    return memcmp(buf1, buf2, size);
}

int osal_strcmp(const char *s1, const char *s2)
{
    return strcmp(s1, s2);
}

int osal_strncmp(const char *s1, const char *s2, unsigned long size)
{
    return strncmp(s1, s2, size);
}

int osal_strcasecmp(const char *s1, const char *s2)
{
    return strcasecmp(s1, s2);
}

int osal_strncasecmp(const char *s1, const char *s2, unsigned long size)
{
    return strncasecmp(s1, s2, size);
}

char *osal_strchr(const char *s, int n)
{
    return strchr(s, n);
}

char *osal_strrchr(const char *s, int c)
{
    return strrchr(s, c);
}

char *osal_strstr(const char *s1, const char *s2)
{
    return strstr(s1, s2);
}

char *osal_strnstr(const char *s1, const char *s2, int len)
{
    return strnstr(s1, s2, len);
}

unsigned int osal_strlen(const char *s)
{
    return strlen(s);
}

unsigned int osal_strnlen(const char *s, unsigned int size)
{
    return strnlen(s, size);
}

char *osal_strpbrk(const char *s1, const char *s2)
{
    return strpbrk(s1, s2);
}

char *osal_strsep(char **s, const char *ct)
{
    return strsep(s, ct);
}

unsigned int osal_strspn(const char *s, const char *accept)
{
    return strspn(s, accept);
}

unsigned int osal_strcspn(const char *s, const char *reject)
{
    return strcspn(s, reject);
}

char *osal_strcpy(char *dest, const char *src)
{
    return strcpy(dest, src);
}

char *osal_strncpy(char *dest, const char *src, unsigned long size)
{
    return strncpy(dest, src, size);
}

int osal_memcmp(const void *cs, const void *ct, int count)
{
    return memcmp(cs, ct, count);
}

void *osal_memchr(const void *s, int c, int n)
{
    return memchr(s, c, n);
}

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

unsigned long osal_strtoul(const char *cp, char **endp, unsigned int base)
{
    return (unsigned long)osal_strtoull(cp, endp, base);
}

long osal_strtol(const char *cp, char **endp, unsigned int base)
{
    return (long)osal_strtoull(cp, endp, base);
}

long long osal_strtoll(const char *cp, char **endp, unsigned int base)
{
    return (long long)osal_strtoull(cp, endp, base);
}