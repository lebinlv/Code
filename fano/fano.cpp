#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <map>
#include <cmath>
#include <iomanip>
#include <string>

using namespace std;

// a fano Node
struct fano_node
{
    char c;      // character.
    unsigned count;   // count of c.
    double cfre; // frequency of c.

    fano_node(char _c, unsigned _count, double _cfre) : c(_c), count(_count), cfre(_cfre) {}
    ~fano_node() {}
};

bool compare_node(fano_node *a, fano_node *b) {return a->count > b->count;}

// 返回文件中的字符总数并为每种字符构建初始节点，如果文件打开失败，返回0
// node_list 为存储初始节点的 vector
unsigned char_count(const char *file_name, list<fano_node*>&node_list)
{
    int temp_array[256] = {0};
    ifstream is (file_name, ifstream::binary);
    if(is) {
        // get length of file
        is.seekg(0, is.end);
        unsigned length = is.tellg();
        is.seekg(0, is.beg);

        char *buffer = new char[length]; // allocate memory
        is.read(buffer, length);         // read data as a block
        is.close();                      // close file

        // count character
        for(unsigned i=0; i<length; i++)
            temp_array[buffer[i] + 128] += 1; 

        delete[] buffer;

        // build node_list
        for (int i = 0; i < 256; i++) {
            if(temp_array[i]>0) {
                fano_node *new_node = new fano_node(char(i-128), temp_array[i], temp_array[i]/double(length));
                node_list.push_back(new_node);
            }
        }
        return length;
    }

    return 0;
}

// 用于计算费诺编码的递归函数
void fano_encode_recursive(int count_sum, list<fano_node *> &node_list_r, map<char, vector<bool>> &code_map, vector<bool> &tmp_vec)
{
    int sum_l = 0;
    list<fano_node *> node_list_l;

    fano_node *temp_node = node_list_r.front();

    // 如果list大小为1，说明已递归到底，得到当前元素的编码，将其保存到 map 中
    if (node_list_r.size() < 2) {
        code_map.insert(make_pair(temp_node->c, tmp_vec));
    }
    // 如果大小大于1，说明还可继续分组
    else {
        // 分为两组
        while ((temp_node->count) < (count_sum - 2 * sum_l)) {
            node_list_l.push_back(temp_node);
            sum_l += temp_node->count;

            node_list_r.pop_front();
            temp_node = node_list_r.front();
        }

        // 对较大的一组进行递归
        tmp_vec.push_back(0); // 分配码元 0
        fano_encode_recursive(sum_l, node_list_l, code_map, tmp_vec);

        // 对另外一组进行递归
        tmp_vec.push_back(1); // 分配码元 1
        fano_encode_recursive(count_sum - sum_l, node_list_r, code_map, tmp_vec);
    }

    // 当前函数结束时，弹出一个码元
    if(tmp_vec.size()>0) tmp_vec.pop_back();
}

// 费诺编码函数，此函数对 node_list 进行排序之后调用递归函数完成编码
void fano_encode(int count_sum, list<fano_node *> node_list, map<char, vector<bool> > &code_map)
{
    vector<bool> tmp_vec;
    node_list.sort(compare_node); // 将 node_list 从大到小排序
    fano_encode_recursive(count_sum, node_list, code_map, tmp_vec);
}

int main()
{
    //char file_name[] = "test.txt";
    //char file_name[] = "GameOfThrones.txt";
    //char file_name[] = "fano.zip";

    double entropy = 0.0,               //信源熵
           average_length = 0.0,        //平均码长
           variance = 0.0,              //码长方差
           efficiency = 0.0;            //编码效率

    list<fano_node *> node_list;        // 存储 node 的 list
    map<char, vector<bool>> code_map;   // 存储费诺编码的 map

    // 获取文件名
    string file_name_s;
    cout << "请输入文件路径：";
    getline(cin, file_name_s);
    const char *file_name = file_name_s.c_str();

    // 统计文件中出现的字符及其出现次数和频率
    unsigned total_char = char_count(file_name, node_list); 

    if (total_char == 0) {
        cout << "ERROR! we count open file or no character founded in the file." << endl;
    } else if (node_list.size() < 2) {
        cout << "ERROR! only one kind of symbol founded in the file!" << endl;
    } else {
        // 进行编码
        fano_encode(total_char, node_list, code_map);

        for (list<fano_node*>::iterator it = node_list.begin(); it != node_list.end(); ++it) {
            // 计算信源熵
            entropy -= ((*it)->cfre) * log2((*it)->cfre);
            // 计算平均码长
            average_length += ((*it)->cfre)*(code_map[(*it)->c].size());
        }

        // 计算码长方差   
        for (list<fano_node*>::iterator it = node_list.begin(); it != node_list.end(); ++it) {
            variance += ((*it)->cfre) * pow((code_map[(*it)->c].size() - average_length), 2);
        }

        // 计算编码效率
        efficiency = entropy / average_length;

        // 输出编码
        cout << " char | ASCII |   count   |  frequency  | code length |          fano code          |" << endl;
        for (list<fano_node *>::iterator it = node_list.begin(); it != node_list.end(); ++it) {
            cout << " '";
            if ((*it)->c == 9) {
                cout << "\\t' | ";
            } else if ((*it)->c == 10) {
                cout << "\\n' | ";
            } else if ((*it)->c == 13) {
                cout << "\\r' | ";
            } else if ((*it)->c >=32 && (*it)->c <= 126) {
                cout << (*it)->c << "'  | ";
            } else {
                cout << "'   | ";
            }
            cout << left << setw(6) << int((*it)->c) << "| ";
            cout << left << setw(10) << (*it)->count << "| ";
            cout << left << setw(12) << scientific << setprecision(3) << (*it)->cfre << "| ";
            cout << left << setw(12) << code_map[(*it)->c].size() << "| ";
            for (const auto &vec_it : (code_map[(*it)->c]))
                cout << vec_it;
            cout << right << setw(29 - code_map[(*it)->c].size()) << '|' << endl;
        }

        cout << fixed << endl;
        cout << "信源熵：" << entropy << endl;
        cout << "平均码长：" << average_length << endl;
        cout << "码长方差：" << variance << endl;
        cout << "编码效率：" << efficiency << endl;
    }

    cout << "press any key to quit..." << endl;

    // 释放空间
    for (list<fano_node*>::iterator it = node_list.begin(); it != node_list.end(); ++it) {
        delete (*it);
    }

    getchar();
    return 0;
}

