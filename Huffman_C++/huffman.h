#ifndef __HUFFMAN_H__
#define __HUFFMAN_H__

#include <iostream>
#include <vector>
#include <array>
#include <map>

class Huffman
{
  public:
    Huffman();
    ~Huffman();

    // 状态代码 无问题， 文件打开失败， 信息源存在问题
    enum huffman_err {HUFFMAN_OK = 0, FILE_OPEN_ERR, SOURCE_ERR};

    huffman_err Encode(const char *); // 为文件编码
    huffman_err Encode(std::string);  // 为字符串编码
    void ShowResult();                // 显示结果，包括编码结果、信源熵、平均码长、码长方差、编码效率等

  private:
    struct tree_node
    {
        char symbol;        // 符号
        double freq;        // 符号的权值（频率）
        unsigned depth;     // 该节点的深度，通过此变量可以保证合并后的新符号排在靠前的位置，使得码长方差最小

        tree_node *L_node;  // 左节点
        tree_node *R_node;  // 右节点

        tree_node(char, double, tree_node *L_node = NULL, tree_node *R_node = NULL);
        ~tree_node();

        class Compare {
          public:
            bool operator()(tree_node *a, tree_node *b) {
                return (a->freq == b->freq ? (a -> depth > b -> depth) : (a->freq > b->freq));
            }
        };
    };

    std::array<unsigned, 256> count_array;                       // 存储符号的出现次数，符号类型为char： -128~127 对应存放在数组的 0~255 位置
    std::vector< std::pair<char, double> > cfre_vector;          // 存储符号及其对应频率的 vector
    std::map< char, std::vector<bool> > huffman_dict;            // 存储符号及其对应编码的 map

    tree_node *huffman_root;        // 霍夫曼树的根节点
    unsigned char_count;            // 总的符号个数
    double entropy;                 // 信源熵
    double ave_length;              // 平均码长
    double sigma2;                  // 码长方差
    double eta;                     // 编码效率

    /**
     * @brief Get the Freq Table object from file
     * @param filename 
     */
    bool GetFreqTable(const char*);

    /**
     * @brief Get the Freq Table object from string
     * @param input_str 
     */
    void GetFreqTable(std::string);

    /**
     * @brief build huffman-tree
     */
    void BuildHuffmanTree();

    /**
     * @brief encode character according to freq
     */
    bool BuildHuffmanDict();

    /**
     * @brief this function that was used by BuildHuffmanDict to encode
     * @param root_node
     * @param tmp_vec: a temp vector
     */
    void BuildHuffmanDictInternal(tree_node *, std::vector<bool> &);

    /**
     * @brief Get the Entropy, Average Length, Sigma^2 and eta.
     */
    void Statistics();
};



#endif