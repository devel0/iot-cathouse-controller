#include "Util.h"

#include <limits.h> // ULONG_MAX

unsigned long timeDiff(unsigned long start, unsigned long now)
{
    if (start <= now)
        return now - start;
    else
        return (ULONG_MAX - start) + now + 1;
}

uint32_t freeMemorySum()
{
    return system_get_free_heap_size();
}

String trimDoubleQuotes(String str)
{
    String res;    

    int strl = str.length();
    int i = 0;

    while (i < strl && str[i] != '"')
        ++i;

    ++i;
    int j = i;
    while (j < strl && str[j] != '"')
        ++j;

    for (int k = i; k < j; ++k)
        res.concat(str[k]);    

    return res;
}

void builtinLedOn()
{
    digitalWrite(LED_BUILTIN, LOW);
}

void builtinLedOff()
{
    digitalWrite(LED_BUILTIN, HIGH);
}