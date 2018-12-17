// https://gist.github.com/arcao/3252bb6e5e52493f03726ec32e61395c

#ifndef _STRING_STREAM_H_
#define _STRING_STREAM_H_

#include <Arduino.h>
#include <Stream.h>

class StringStream : public Stream
{
  public:
    StringStream(String &s) : str(s), position(0) {}

    // Stream methods
    virtual int available() { return str.length() - position; }
    virtual int read() { return position < str.length() ? str[position++] : -1; }
    virtual int peek() { return position < str.length() ? str[position] : -1; }
    virtual void flush(){};
    // Print methods
    virtual size_t write(uint8_t c)
    {
        str += (char)c;        
        return 1;
    };

  private:
    String &str;
    unsigned int length;
    unsigned int position;
};

#endif // _STRING_STREAM_H_
