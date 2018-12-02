#include <fstream>
#include <queue>
#include <cmath>
#include <iomanip>

#include "huffman.h"

using namespace std;

Huffman::encode_tree_node::encode_tree_node(uint8_t _symbol, uint32_t _count, encode_tree_node *_L_node, encode_tree_node *_R_node) :
                              symbol(_symbol), count(_count), L_node(_L_node), R_node(_R_node) {
    if (L_node && R_node)
        depth = L_node->depth > R_node->depth ? L_node->depth + 1 : R_node->depth + 1;
    else
        depth = 0;
}

/**
 * @brief 从文件中统计各符号的出现次数
 */
bool Huffman::GetFreqTable(const char *filename)
{
    char buffer[65536];
    uint8_t *u8_buffer = (uint8_t *)buffer;
    char_count = 0;

    ifstream infile(filename, ifstream::in | ifstream::binary);

    if(infile) {
        do {
            unsigned i;
            infile.read((char *)buffer, 65536);
            for (i = 0; i < infile.gcount(); i++) {
                symbol_array[u8_buffer[i]].count += 1;
            }
            char_count += i;
        } while (infile);

        for (unsigned i = 0; i < 256; i++) {
            symbol_array[i].freq = symbol_array[i].count / double(char_count);
        }
        infile.close();
    }
    return char_count;
}

/**
 * @brief 从字符串中统计各符号的出现次数
 */
bool Huffman::GetFreqTable(string input_str)
{
    char_count = input_str.size();

    for (unsigned i = 0; i < char_count; i++) {
        symbol_array[uint8_t(input_str[i])].count += 1;
    }

    for (unsigned i = 0; i < 256; i++) {
        symbol_array[i].freq = symbol_array[i].count / double(char_count);
    }

    return char_count;
}

/**
 * @brief 构造霍夫曼树
 * @return 有多少种类的符号
 */
uint32_t Huffman::BuildHuffmanTree()
{
    uint32_t kind_of_symbol = 0;
    // 初始化树节点，并将其放到优先级队列中，保证每次弹出的都是权值最小的（若权值相等则弹出深度较小的）
    priority_queue< encode_tree_node*, vector< encode_tree_node*>, encode_tree_node::Compare > node_queue;

    for (unsigned i = 0; i < 256; i++) {
        if (symbol_array[i].count) {
            encode_tree_node *node = new encode_tree_node(i, symbol_array[i].count);
            node_queue.push(node);
            kind_of_symbol ++;
        }
    }

    encode_tree_node *root = NULL;
    while(node_queue.size() > 1) {
        // 取出最小的作为左子树
        encode_tree_node *left = node_queue.top();
        node_queue.pop();

        // 取出次小的作为右子树
        encode_tree_node *right = node_queue.top();
        node_queue.pop();

        // 创建新树
        root = new encode_tree_node(0, left->count + right->count, left, right);

        // 将新树放入到队列中
        node_queue.push(root);
    }
    delete huffman_root; // 删除旧的根节点以保证所有通过new申请的空间得到释放
    huffman_root = root; // 新的根节点

    return kind_of_symbol;
}

/**
 * @brief 先序遍历霍夫曼树以进行霍夫曼编码，该函数被 BuildHuffmanDict 调用 
 */
void Huffman::BuildHuffmanDictInternal(encode_tree_node *root_node, uint32_t code, uint32_t bits, vector<char> &tmp_vec)
{
    uint8_t leaf_symbol;
    if(root_node->L_node == NULL && root_node->R_node == NULL) {
        leaf_symbol = root_node->symbol;

        symbol_array[leaf_symbol].code = code;
        symbol_array[leaf_symbol].bits = bits;
        symbol_array[leaf_symbol].binary_code = new char[tmp_vec.size()];
        memcpy(symbol_array[leaf_symbol].binary_code, &tmp_vec[0], tmp_vec.size());

        encode_stream.writbits(1, 1);
        encode_stream.writbits(root_node->symbol, 8);
    } else {
        encode_stream.writbits(0, 1);

        tmp_vec.push_back(1); // 分配码元 1
        BuildHuffmanDictInternal(root_node->L_node, (code << 1) + 1, bits + 1, tmp_vec);
        tmp_vec.push_back(0);  // 分配码元 0
        BuildHuffmanDictInternal(root_node->R_node, (code << 1) + 0, bits + 1, tmp_vec);
    }

    if(!tmp_vec.empty()) {
        tmp_vec.pop_back();
    }
}

/**
 * @brief 霍夫曼编码的主函数，并把霍夫曼树信息保存到输出流中，该函数调用 BuildHuffmanDictInternal 实现编码
 */
void Huffman::BuildHuffmanDict()
{
    std::vector<char> code_vec;
    BuildHuffmanDictInternal(huffman_root, 0, 0, code_vec);
}

void Huffman::Statistics()
{
    entropy = 0.0;
    ave_length = 0.0;
    for (unsigned i = 0; i < 256; i++) {
        if (symbol_array[i].count) {
            // 计算信源熵
            entropy -= symbol_array[i].freq * log2(symbol_array[i].freq);
            // 计算平均码长
            ave_length += symbol_array[i].freq * symbol_array[i].bits;
        }
    }

    // 计算码长方差
    variance = 0.0;
    for (unsigned i = 0; i < 256; i++) {
        if (symbol_array[i].count) {
            variance += symbol_array[i].freq * pow((symbol_array[i].bits - ave_length), 2);
        }
    }

    // 计算编码效率
    efficiency = entropy / ave_length;
}

Huffman::huffman_err Huffman::Encode(const char filename[])
{
    if(!GetFreqTable(filename))  return FILE_OPEN_ERR;  // 统计频率
    if(BuildHuffmanTree() < 2) return SOURCE_ERR;       // 构建霍夫曼树
    BuildHuffmanDict();                                 // 遍历树进行编码
    Statistics();                                       // 统计各项指标

    return HUFFMAN_OK;
}

Huffman::huffman_err Huffman::Encode(std::string usr_str)
{
    GetFreqTable(usr_str);                        // 统计频率
    if(BuildHuffmanTree() < 2) return SOURCE_ERR; // 构建霍夫曼树
    BuildHuffmanDict();                           // 遍历树进行编码
    Statistics();                                 // 统计各项指标

    return HUFFMAN_OK;
}

Huffman::huffman_err Huffman::compress(const char *src_file, const char *dst_file)
{
    // 创建压缩后的文件
    if(!encode_stream.open(dst_file)) return DST_ERR;

    // 计算在文件最后需要补多少个0
    uint8_t last_bits = (11 - encode_stream.freebits) % 8;
    for (unsigned i = 0; i < 256; i++) {
        last_bits = (last_bits + symbol_array[i].count * symbol_array[i].bits) % 8;
    }
    if(last_bits) {
        encode_stream.writbits(8 - last_bits, 3);
    } else encode_stream.writbits(0, 3);

    // 打开源文件，逐字节进行压缩
    char buffer[65536];
    uint8_t symbol;
    ifstream infile(src_file, ifstream::in | ifstream::binary);
    if(infile) {
        do {
            unsigned i;
            infile.read((char *)buffer, 65536);
            for (i = 0; i < infile.gcount(); i++) {
                symbol = buffer[i];
                encode_stream.writbits(symbol_array[symbol].code, symbol_array[symbol].bits);
            }
        } while (infile);
        infile.close();
    }
    encode_stream.close();

    return HUFFMAN_OK;
}

Huffman::huffman_err Huffman::compress(std::string &src_str, const char *dst_file)
{
    // 创建压缩后的文件
    if (!encode_stream.open(dst_file)) return DST_ERR;

    // 计算在文件最后需要补多少个0
    uint8_t last_bits = (11 - encode_stream.freebits) % 8;
    for (unsigned i = 0; i < 256; i++) {
        last_bits = (last_bits + symbol_array[i].count * symbol_array[i].bits) % 8;
    }
    if (last_bits) {
        encode_stream.writbits(8 - last_bits, 3);
    } else encode_stream.writbits(0, 3);

    // 逐字节进行压缩
    uint8_t symbol;
    for (unsigned i = 0; i < char_count; i++) {
        symbol = src_str[i];
        encode_stream.writbits(symbol_array[symbol].code, symbol_array[symbol].bits);
    }
    encode_stream.close();

    return HUFFMAN_OK;
}

void Huffman::RecoverTree(ibitstream &decode_stream, decode_tree_node *node)
{
    if(decode_stream.readbit()) {
        node->symbol = decode_stream.read8bits();
        return;
    }

    node->L_node = new decode_tree_node();
    RecoverTree(decode_stream, node->L_node);

    node->R_node = new decode_tree_node();
    RecoverTree(decode_stream, node->R_node);
}

Huffman::huffman_err Huffman::decompress(const char *src_file, const char *dst_file)
{
    // 打开待解压的文件
    ibitstream decode_stream;
    if(!decode_stream.open(src_file)) return SOURCE_ERR;

    // 创建解压后的文件
    obitstream decompress_stream;
    if(!decompress_stream.open(dst_file)) return DST_ERR;

    // 从文件头部信息中重建霍夫曼树
    decode_tree_node root_node;
    RecoverTree(decode_stream, &root_node);

    // 读取文件末尾补的0的个数
    uint8_t zero_padding;
    zero_padding = decode_stream.readbit() << 2;
    zero_padding |= decode_stream.readbit() << 1;
    zero_padding |= decode_stream.readbit();

    // 解压缩
    decode_tree_node *node;
    while(decode_stream.remain_bits > zero_padding) {
        node = &root_node;
        while(node->L_node && node->R_node) {
            node = decode_stream.readbit() ? node->L_node : node->R_node;
        }
        decompress_stream.writbyte((uint8_t)(node->symbol));
    }

    decompress_stream.close();
    decode_stream.close();

    return HUFFMAN_OK;
}

void Huffman::ShowResult()
{
    char  line[] = "+-------------+---------+-----------------+-------------+----------------------------+";
    char title[] = "|   Symbol    |  Count  |    Frequency    | Code Length |            Code            |";
    char c7_13[][6] = { "\\a'  ", "\\b'  ", "\\t'  ", "\\n'  ", "\\v'  ", "\\f'  ", "\\r'  "};
    char symbol;

    cout << line << endl;
    cout << title << endl;
    cout << line << endl;

    unsigned idx = 0;
    for (unsigned i = 0; i < 256; i++)
    {
        if (symbol_array[i].count > 0)
        {
            symbol = char(i);
            cout << "| ";
            cout << right << setw(4) << int(symbol) << " '";
            if (symbol < 7 ) {
                cout << "'    ";
            } else if (symbol < 14) {
                cout << c7_13[symbol-7];
            } else if (symbol < 32) {
                cout << "'    ";
            } else if (symbol < 127) {
                cout << symbol << "'   "; 
            } else {
                cout << "'    ";
            }
            cout << " | ";
            cout << left << setw(8) << symbol_array[i].count << "| ";
            cout << left << setw(15) << scientific << setprecision(3) << symbol_array[i].freq << " | ";
            cout << left << setw(11) << int(symbol_array[i].bits) << " | ";
            for (unsigned j = 0; j < symbol_array[i].bits; j++)
                cout << int(symbol_array[i].binary_code[j]);
            cout << right << setw(28 - symbol_array[i].bits) << '|' << endl;

            cout << line << endl;
        }
    }
    cout << "Symbols Count: ";
    cout << char_count << endl;

    cout << "Entropy: ";
    cout << fixed;
    cout << left << setw(15) << setprecision(6) << entropy;
    cout << "Average Length: ";
    cout << left << setw(12) << setprecision(6) << ave_length;
    cout << "Code Variance: ";
    cout << left << setw(12) << setprecision(6) << variance;
    cout << "Efficiency: ";
    cout << left << setw(12) << setprecision(6) << efficiency;

    cout << endl;
}