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

// �����ļ��е��ַ�������Ϊÿ���ַ�������ʼ�ڵ㣬����ļ���ʧ�ܣ�����0
// node_list Ϊ�洢��ʼ�ڵ�� vector
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

// ���ڼ����ŵ����ĵݹ麯��
void fano_encode_recursive(int count_sum, list<fano_node *> &node_list_r, map<char, vector<bool>> &code_map, vector<bool> &tmp_vec)
{
    int sum_l = 0;
    list<fano_node *> node_list_l;

    fano_node *temp_node = node_list_r.front();

    // ���list��СΪ1��˵���ѵݹ鵽�ף��õ���ǰԪ�صı��룬���䱣�浽 map ��
    if (node_list_r.size() < 2) {
        code_map.insert(make_pair(temp_node->c, tmp_vec));
    }
    // �����С����1��˵�����ɼ�������
    else {
        // ��Ϊ����
        while ((temp_node->count) < (count_sum - 2 * sum_l)) {
            node_list_l.push_back(temp_node);
            sum_l += temp_node->count;

            node_list_r.pop_front();
            temp_node = node_list_r.front();
        }

        // �Խϴ��һ����еݹ�
        tmp_vec.push_back(0); // ������Ԫ 0
        fano_encode_recursive(sum_l, node_list_l, code_map, tmp_vec);

        // ������һ����еݹ�
        tmp_vec.push_back(1); // ������Ԫ 1
        fano_encode_recursive(count_sum - sum_l, node_list_r, code_map, tmp_vec);
    }

    // ��ǰ��������ʱ������һ����Ԫ
    if(tmp_vec.size()>0) tmp_vec.pop_back();
}

// ��ŵ���뺯�����˺����� node_list ��������֮����õݹ麯����ɱ���
void fano_encode(int count_sum, list<fano_node *> node_list, map<char, vector<bool> > &code_map)
{
    vector<bool> tmp_vec;
    node_list.sort(compare_node); // �� node_list �Ӵ�С����
    fano_encode_recursive(count_sum, node_list, code_map, tmp_vec);
}

int main()
{
    //char file_name[] = "test.txt";
    //char file_name[] = "GameOfThrones.txt";
    //char file_name[] = "fano.zip";

    double entropy = 0.0,               //��Դ��
           average_length = 0.0,        //ƽ���볤
           variance = 0.0,              //�볤����
           efficiency = 0.0;            //����Ч��

    list<fano_node *> node_list;        // �洢 node �� list
    map<char, vector<bool>> code_map;   // �洢��ŵ����� map

    // ��ȡ�ļ���
    string file_name_s;
    cout << "�������ļ�·����";
    getline(cin, file_name_s);
    const char *file_name = file_name_s.c_str();

    // ͳ���ļ��г��ֵ��ַ�������ִ�����Ƶ��
    unsigned total_char = char_count(file_name, node_list); 

    if (total_char == 0) {
        cout << "ERROR! we count open file or no character founded in the file." << endl;
    } else if (node_list.size() < 2) {
        cout << "ERROR! only one kind of symbol founded in the file!" << endl;
    } else {
        // ���б���
        fano_encode(total_char, node_list, code_map);

        for (list<fano_node*>::iterator it = node_list.begin(); it != node_list.end(); ++it) {
            // ������Դ��
            entropy -= ((*it)->cfre) * log2((*it)->cfre);
            // ����ƽ���볤
            average_length += ((*it)->cfre)*(code_map[(*it)->c].size());
        }

        // �����볤����   
        for (list<fano_node*>::iterator it = node_list.begin(); it != node_list.end(); ++it) {
            variance += ((*it)->cfre) * pow((code_map[(*it)->c].size() - average_length), 2);
        }

        // �������Ч��
        efficiency = entropy / average_length;

        // �������
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
        cout << "��Դ�أ�" << entropy << endl;
        cout << "ƽ���볤��" << average_length << endl;
        cout << "�볤���" << variance << endl;
        cout << "����Ч�ʣ�" << efficiency << endl;
    }

    cout << "press any key to quit..." << endl;

    // �ͷſռ�
    for (list<fano_node*>::iterator it = node_list.begin(); it != node_list.end(); ++it) {
        delete (*it);
    }

    getchar();
    return 0;
}

