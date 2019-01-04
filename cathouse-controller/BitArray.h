#ifndef _BIT_ARRAY_H_
#define _BIT_ARRAY_H_

#include <Arduino.h>

class BitArray
{
  int capacity;
  uint8_t *arr;

public:
  BitArray(int capacity);
  ~BitArray();
  
  int Capacity() const;
  bool Get(int idx) const;
  void Set(int idx, bool value);
};

#endif