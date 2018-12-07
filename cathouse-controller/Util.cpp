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