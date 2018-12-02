#include <cstring>
#include "bitstream.h"

using namespace std;

/*************************************************************************
*  class obitstream
*************************************************************************/

obitstream::obitstream()
{
    memset(buffer, 0, BIT_STREAM_BUFFER_LEHGTH);
    pByte = buffer;
    freebits = 8;
}

bool obitstream::writbits(uint32_t x, uint8_t bits)
{
    while (bits /*&& (pByte - buffer > 65536)*/)
    {
        if(freebits < bits) {
            *pByte |= (x >> (bits - freebits));
            bits -= freebits;
            freebits = 0;
        } else {
            *pByte |= (x << (freebits - bits));
            freebits -= bits;
            bits = 0;
        }

        // 如果当前字节已经写满
        if(!freebits) {
            pByte++;        // 指向下一个字节
            freebits = 8;   // 置空闲位=8

            // 如果缓冲区已满
            if (pByte - buffer >= BIT_STREAM_BUFFER_LEHGTH ) {
                //if(ofs.is_open()) {    // 如果输出文件已经打开，则将缓存区写入文件
                                         // 由于huffman.cpp中的函数在压缩前将文件打开，所以将此项注释以优化压缩速度
                ofs.write((char *)buffer, BIT_STREAM_BUFFER_LEHGTH);
                memset(buffer, 0, BIT_STREAM_BUFFER_LEHGTH);
                pByte = buffer;
                //}else return false;
            }
        }
    }
    return true;
}

void obitstream::writbyte(uint8_t x)
{
    *pByte = x;
    ++ pByte;
    if (pByte - buffer >= BIT_STREAM_BUFFER_LEHGTH) {
        //if(ofs.is_open()) {    // 如果输出文件已经打开，则将缓存区写入文件
        // 由于huffman.cpp中的函数在压缩前将文件打开，所以将此项注释以优化压缩速度
        ofs.write((char *)buffer, BIT_STREAM_BUFFER_LEHGTH);
        // memset(buffer, 0, BIT_STREAM_BUFFER_LEHGTH);
        pByte = buffer;
        //}else return false;
    }
}

bool obitstream::open(const char filename[])
{
    ofs.open(filename, ofstream::out | ofstream::binary);
    return ofs.is_open();
}

void obitstream::close()
{
    // 将缓冲区剩余内容写入文件
    uint32_t length = (freebits == 8) ? (pByte - buffer) : (pByte - buffer + 1);
    ofs.write((char *)buffer, length);

    // 关闭文件
    ofs.close();
}


/*************************************************************************
*  class ibitstream
*************************************************************************/

bool ibitstream::open(const char filename[])
{
    ifs.open(filename, ifstream::in | ifstream::binary);
    if(ifs.is_open()) {
        ifs.read(&buffer[1], BIT_STREAM_BUFFER_LEHGTH);
        remain_bits = ifs.gcount()*8;
        return true;
    }
    return false;
}

void ibitstream::close()
{
    ifs.close();
}

uint8_t ibitstream::readbit()
{
    uint8_t x = (*pByte & (1<<(7-bitpos))) ? 1:0;
    bitpos += 1;
    remain_bits -= 1;

    if(bitpos > 7) {
        bitpos = 0;
        ++ pByte;
    }

    if(remain_bits < 9 && ifs) {
        buffer[0] = *pByte;
        pByte = (uint8_t*)buffer;
        ifs.read(&buffer[1], BIT_STREAM_BUFFER_LEHGTH);
        remain_bits = ifs.gcount() * 8 + 8;
    }
    return x;
}


uint8_t ibitstream::read8bits()
{
    uint8_t x;
    uint8_t *buf = pByte;

    x = bitpos ? ((buf[0] << bitpos) | (buf[1] >> (8 - bitpos))) : *buf;

    ++ pByte;
    remain_bits -= 8;

    return x;
}
