#include <fstream>
#include <queue>
#include <cmath>
#include <iomanip>

#include "huffman.h"

using namespace std;

Huffman::Huffman()
{
    count_array.fill(0);
    huffman_root = NULL;
    char_count = 0;
    entropy = 0.0;
    ave_length = 0.0;
    sigma2 = 0.0;
    eta = 0.0;
}

Huffman::~Huffman()
{
    delete huffman_root;
}

Huffman::tree_node::tree_node(char symbol, double freq, tree_node *L_node, tree_node *R_node) :
                              symbol(symbol), freq(freq), L_node(L_node), R_node(R_node) {
    if (L_node && R_node)
        depth = L_node->depth > R_node->depth ? L_node->depth + 1 : R_node->depth + 1;
    else
        depth = 0;
}

Huffman::tree_node::~tree_node()
{
    delete L_node, delete R_node;
};


/**
 * @brief Get the Freq Table from file
 * 
 * @param filename 
 * @param cfre_vector 
 * @return unsigned 
 */
bool Huffman::GetFreqTable(const char filename[])
{
    char buffer[65536];
    count_array.fill(0);
    char_count = 0;

    cfre_vector.clear();

    ifstream infile(filename, ifstream::in | ifstream::binary);

    if(infile) {
        do {
            unsigned i = 0;
            infile.read((char *)buffer, 65536);

            for (i = 0; i < infile.gcount(); i++) {
                count_array[buffer[i] + 128] += 1; // 使用数组进行统计
            }
            char_count += i;
        } while (infile);

        for (unsigned i = 0; i < 256; i++) {
            if (count_array[i] > 0) {
                cfre_vector.push_back(make_pair(char(i - 128), count_array[i] / double(char_count)));
            }
        }
        infile.close();
        return true;
    }
    
    return false;
}

/**
 * @brief Get the Freq Table from string
 * 
 * @param input_str 
 * @param cfre_vector 
 */
void Huffman::GetFreqTable(string input_str)
{
    count_array.fill(0);
    char_count = input_str.size();
    cfre_vector.clear();

    for (size_t i = 0; i < char_count; i++) {
        count_array[input_str[i] +128] += 1;
    }

    for (unsigned i = 0; i < 256; i++) {
        if (count_array[i] > 0) {
            cfre_vector.push_back(make_pair(char(i - 128), count_array[i] / double(char_count)));
        }
    }
}

/**
 * @brief 创建霍夫曼树
 * 
 * @param cfre_vector   vector of (character, frequency) pairs
 * @return tree_node*   the address of huffman tree's root node
 */
void Huffman::BuildHuffmanTree()
{
    // 初始化树节点，并将其放到优先级队列中，保证每次弹出的都是权值最小的（若权值相等则弹出深度较小的）
    priority_queue< tree_node*, vector< tree_node*>, tree_node::Compare > node_queue;
    for (vector< pair<char, double> >::iterator it = cfre_vector.begin();
         it != cfre_vector.end(); ++it) {
        tree_node *node = new tree_node(it->first, it->second);
        node_queue.push(node);
    }

    tree_node *root = NULL;
    while(node_queue.size() > 1) {
        // 取出最小的作为左子树
        tree_node *left = node_queue.top();
        node_queue.pop();

        // 取出次小的作为右子树
        tree_node *right = node_queue.top();
        node_queue.pop();

        // 创建新树
        root = new tree_node(0, left->freq + right->freq, left, right);

        // 将新树放入到队列中
        node_queue.push(root);
    }
    delete huffman_root; // 删除旧的根节点以保证所有通过new申请的空间得到释放
    huffman_root = root; // 新的根节点
}

/**
 * @brief 使用递归函数遍历霍夫曼树以实现霍夫曼编码，该函数被 BuildHuffmanDict 调用
 * 
 * @param root_node 
 * @param tmp_vec 
 */
void Huffman::BuildHuffmanDictInternal(tree_node *root_node, vector<bool> &tmp_vec)
{
    if(root_node->L_node == NULL && root_node->R_node == NULL) {
        huffman_dict.insert(make_pair(root_node->symbol, tmp_vec));
    } else {
        tmp_vec.push_back(1); // 分配码元 1
        BuildHuffmanDictInternal(root_node->L_node, tmp_vec);
        tmp_vec.push_back(0);  // 分配码元 0
        BuildHuffmanDictInternal(root_node->R_node, tmp_vec);
    }
    if(!tmp_vec.empty()) {
        tmp_vec.pop_back();
    }
}

/**
 * @brief 霍夫曼编码的主函数，该函数调用 BuildHuffmanDictInternal 进行递归
 * 
 * @return true 
 * @return false 
 */
bool Huffman::BuildHuffmanDict()
{
    std::vector<bool> code_vec;
    huffman_dict.clear();

    if(cfre_vector.size()>1) {
        BuildHuffmanDictInternal(huffman_root, code_vec);
        return true;
    } else {
        return false;
    }
}

void Huffman::Statistics()
{
    entropy = 0.0;
    ave_length = 0.0;
    for (vector<pair<char, double>>::iterator it = cfre_vector.begin();
         it != cfre_vector.end(); ++it) {

        // 计算信源熵
        entropy -= (it->second) * log2(it->second);

        // 计算平均码长
        ave_length += (it->second) * (huffman_dict[(it->first)]).size();
    }

    // 计算码长方差
    sigma2 = 0.0;
    for (vector<pair<char, double>>::iterator it = cfre_vector.begin();
         it != cfre_vector.end(); ++it) {
        sigma2 += (it->second) * pow(((huffman_dict[(it->first)]).size() - ave_length), 2);
    }

    // 计算编码效率
    eta = entropy / ave_length;
}

Huffman::huffman_err Huffman::Encode(const char filename[])
{
    // 统计频率
    if(!GetFreqTable(filename)) 
        return FILE_OPEN_ERR;
    // 建树
    BuildHuffmanTree();
    // 遍历树进行编码
    if(!BuildHuffmanDict())
        return SOURCE_ERR;
    // 统计各项指标
    Statistics();
    return HUFFMAN_OK;
}

Huffman::huffman_err Huffman::Encode(std::string usr_str)
{
    GetFreqTable(usr_str);
    BuildHuffmanTree();
    if(!BuildHuffmanDict())
        return SOURCE_ERR;
    Statistics();
    return HUFFMAN_OK;
}

void Huffman::ShowResult()
{
    char  line[] = "+-------------+---------+-----------------+-------------+----------------------------+";
    char title[] = "|   Symbol    |  Count  |    Frequency    | Code Length |            Code            |";
    char c7_13[][6] = { "\\a'  ", "\\b'  ", "\\t'  ", "\\n'  ", "\\v'  ", "\\f'  ", "\\r'  "};

    cout << line << endl;
    cout << title << endl;
    cout << line <<endl;

    unsigned idx = 0;
    for (map<char, vector<bool>>::iterator it = huffman_dict.begin();
         it != huffman_dict.end(); ++it, idx++)
    {
        cout << "| ";
        cout << right << setw(4) << int(it->first) << " '";
        if (it->first < 7 ) {
            cout << "'    ";
        } else if (it->first < 14) {
            cout << c7_13[it->first-7];
        } else if (it->first < 32) {
            cout << "'    ";
        } else if (it->first < 127) {
            cout << it->first << "'   "; 
        } else {
            cout << "'    ";
        }
        cout << " | ";
        cout << left << setw(8) << count_array[it->first+128] << "| ";
        cout << left << setw(15) << scientific << setprecision(3) << cfre_vector[idx].second << " | ";
        cout << left << setw(11) << it->second.size() << " | ";
        for (const auto &vec_it : (it->second))
            cout << vec_it;
        cout << right << setw(28-it->second.size()) << '|' << endl;

        cout << line << endl;
    }
    cout << "Symbols Count: ";
    cout << char_count << endl;

    cout << "Entropy: ";
    cout << fixed;
    cout << left << setw(15) << setprecision(6) << entropy;
    cout << "Average Length: ";
    cout << left << setw(12) << setprecision(6) << ave_length;
    cout << "Code Variance: ";
    cout << left << setw(12) << setprecision(6) << sigma2;
    cout << "Efficiency: ";
    cout << left << setw(12) << setprecision(6) << eta;

    cout << endl;
}