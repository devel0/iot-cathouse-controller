#include <limits.h>

unsigned long TimeDiff(unsigned long start, unsigned long now)
{
    if (start <= now)
        return now - start;
    else
        return (ULONG_MAX - start) + now + 1;
}