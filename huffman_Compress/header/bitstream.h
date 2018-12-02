#ifndef _BITSTREAM_H_
#define _BITSTREAM_H_

#include <iostream>
#include <fstream>

#define BIT_STREAM_BUFFER_LEHGTH 65536

// 为保证数据安全，在使用 writbits 函数之前尽量首先使用 open 函数打开文件
// 类中并不提供保护！！！
class obitstream
{
  public:
    obitstream();
    ~obitstream(){}

    uint8_t freebits;

    bool writbits(uint32_t x, uint8_t bits);
    void writbyte(uint8_t x);
    bool open(const char *filename);
    void close();

  private:
    uint8_t buffer[BIT_STREAM_BUFFER_LEHGTH];
    uint8_t *pByte;
    std::ofstream ofs;
};

class ibitstream
{
  public:
    ibitstream() : bitpos(0) { pByte = (uint8_t*)(&buffer[1]); }
    ~ibitstream(){}

    uint8_t readbit();
    uint8_t read8bits();
    bool open(const char filename[]);
    void close();
    uint32_t remain_bits;

  private:
    char buffer[BIT_STREAM_BUFFER_LEHGTH+1];
    uint8_t bitpos;
    uint8_t *pByte;
    std::ifstream ifs;
};

#endif