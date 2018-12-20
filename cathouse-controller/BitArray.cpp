#include "BitArray.h"

int ipow(int b, int exp)
{
    int res = 1;
    while (exp--)
        res *= b;
    return res;
}

BitArray::BitArray(int capacity)
{
    this->capacity = capacity;
    arr = new uint8_t[capacity / 8];
}

BitArray::~BitArray()
{
    delete arr;
}

int BitArray::Capacity() const
{
    return capacity;
}

bool BitArray::Get(int idx) const
{
    return arr[idx / 8] & ipow(2, idx % 8);
}

void BitArray::Set(int idx, bool value)
{
    if (Get(idx))
    {
        if (!value)
            arr[idx / 8] -= ipow(2, idx % 8);
    }
    else
    {
        if (value)
            arr[idx / 8] += ipow(2, idx % 8);
    }
}