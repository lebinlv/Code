#ifndef __HUFFMAN_H__
#define __HUFFMAN_H__

#include <iostream>
#include <vector>

#include "bitstream.h"

class Huffman
{
  public:
    Huffman() : huffman_root(nullptr) {}
    ~Huffman() { delete huffman_root; }

    unsigned char_count; // 总的符号个数
    double entropy;      // 信源熵
    double ave_length;   // 平均码长
    double variance;     // 码长方差
    double efficiency;   // 编码效率

    //状态代码    HUFFMAN_OK:无问题   FILE_OPEN_ERR:文件打开失败   SOURCE_ERR:信息源存在问题
    enum huffman_err { HUFFMAN_OK = 0, FILE_OPEN_ERR, SOURCE_ERR, DST_ERR };

    /**
     * @brief 打开文件并进行霍夫曼编码
     * 
     * @param src_file      - 待编码的文件
     * @return huffman_err  - 函数执行结果
     */
    huffman_err Encode(const char *src_file);

    /**
     * @brief 根据输入的字符串进行霍夫曼编码
     * 
     * @return huffman_err 函数执行结果
     */
    huffman_err Encode(std::string);

    /**
     * @brief 对文件进行压缩，该函数必须在 Encode(const char *) 函数后调用
     * 
     * @param src_file  - 源文件名
     * @param dst_file  - 压缩后的文件名
     */
    huffman_err compress(const char *src_file, const char *dst_file);

    /**
     * @brief 对字符串进行压缩，该函数必须在 Encode(std::string) 函数后调用
     * 
     * @param src_str   - 源字符串
     * @param dst_file  - 压缩后的文件
     */
    huffman_err compress(std::string &src_str, const char *dst_file);

    /**
     * @brief 解压缩
     * 
     * @param src_file  - 待解压缩的文件名
     * @param dst_file  - 压缩后的文件名
     */
    huffman_err decompress(const char *src_file, const char *dst_file);

    /**
     * @brief 显示结果，包括编码结果、信源熵、平均码长、码长方差、编码效率等
     */
    void ShowResult();

  private:
    // 霍夫曼树节点结构体
    struct encode_tree_node
    {
        uint8_t symbol;     // 该节点中的符号, 把符号当作 uint8类型
        uint32_t count;     // 符号出现的次数(该节点的权重)
        uint8_t  depth;     // 该节点的深度，通过此变量可以保证码长方差最小

        encode_tree_node *L_node;  // 左子节点指针
        encode_tree_node *R_node;  // 右子节点指针

        encode_tree_node(uint8_t _symbol, uint32_t _count,
                         encode_tree_node *_L_node = nullptr, encode_tree_node *_R_node = nullptr);
        ~encode_tree_node() { delete L_node, delete R_node; };

        // 用于比较节点的优先级, 两两比较时，权重小的优先级大，若权重相等，则深度小的优先级大
        class Compare {
          public:
            bool operator()(encode_tree_node *a, encode_tree_node *b) {
                return (a->count == b->count ? (a -> depth > b -> depth) : (a->count > b->count));
            }
        };
    };

    struct decode_tree_node
    {
        char symbol;
        decode_tree_node *L_node;
        decode_tree_node *R_node;

        decode_tree_node() : symbol(0), L_node(nullptr), R_node(nullptr) {}
        ~decode_tree_node(){ delete L_node, delete R_node; }
    };

    // 符号结构体
    struct symbol_t
    {
        uint32_t count;       // 该符号出现的次数
        double    freq;       // 该符号的频率
        uint32_t  code;       // 该符号的霍夫曼编码，整型类型(32位整型，所以编码最长32位，即树的深度最大为32)
        uint8_t   bits;       // 该符号的编码长度
        char *binary_code;    // 该符号的霍夫曼编码，用 0, 1 直观的表示

        symbol_t() : count(0), freq(0.0), code(0), bits(0), binary_code(nullptr) {}
        ~symbol_t() { delete [] binary_code; }
    };

    // 由于把符号当作 uint8类型对待，所以最多有256种符号， 用数组来存储可以保证访问速度；
    symbol_t symbol_array[256];

    obitstream encode_stream;
    encode_tree_node *huffman_root; // 霍夫曼树的根节点

    /**
     * @brief 从文件中统计各符号的出现次数
     */
    bool GetFreqTable(const char*);

    /**
     * @brief 从字符串中统计各符号的出现次数
     */
    bool GetFreqTable(std::string);

    /**
     * @brief build huffman-tree
     */
    uint32_t BuildHuffmanTree();

    /**
     * @brief 霍夫曼编码的主函数，并把霍夫曼树信息保存到输出流中，该函数调用 BuildHuffmanDictInternal 实现编码
     */
    void BuildHuffmanDict();

    /**
     * @brief 先序遍历霍夫曼树以进行霍夫曼编码，该函数被 BuildHuffmanDict 调用 
     */
    void BuildHuffmanDictInternal(encode_tree_node *, uint32_t, uint32_t, std::vector<char> &);

    /**
     * @brief 计算信源熵、平均码长、码长方差、编码效率
     */
    void Statistics();

    /**
     * @brief 从压缩文件中重建霍夫曼树
     */
    void RecoverTree(ibitstream &, decode_tree_node *);
};

#endif