#include <iostream>
#include <string>
#include <iomanip>
#include <windows.h>

#include "huffman.h"
#include "huffman_ui.h"

using namespace std;

/*************************************************************************
* 私有函数的前向声明
*************************************************************************/
void _welcome();
unsigned _menu(string &input);
void _compress(Huffman *code, std::string src, bool FileOrStr);
void _de_compress(Huffman *code, std::string &src);
void _encode(Huffman *code, std::string src, bool FileOrStr);


/*************************************************************************
* public function
*************************************************************************/

// 命令行模式
void CommandMode(char *argv[])
{
    string src;
    Huffman code;
    if(argv[1][1] == 'f') {
        src = argv[2];
        _encode(&code, src, 1);
    } else if(argv[1][1] == 's') {
        src = argv[2];
        _encode(&code, src, 0);
    } else if(argv[1][1] == 'u') {
        src = argv[2];
        _de_compress(&code, src);
    }
    else {
        std::cout << "Usage: " << argv[0] << " [-?] [-h] [-f xxx] [-s xxx]" << endl;
        std::cout << "    " << left << setw(10) << "-?";
        std::cout << "Display help." << endl;
        std::cout << "    " << left << setw(10) << "-h";
        std::cout << "Display help." << endl;
        std::cout << "    " << left << setw(10) << "-f xxx";
        std::cout << "treat xxx as file path and encode the file." << endl;
        std::cout << "    " << left << setw(10) << "-s xxx";
        std::cout << "treat xxx as string and encode it." << endl;
        std::cout << "    " << left << setw(10) << "-u xxx";
        std::cout << "treat xxx as compressed file and decompress it." << endl;
    }
}

// 独立模式
void IndependenceMode()
{
    string src;
    Huffman *code;
    const char *file_name = NULL;
    unsigned choice = 0;

    _welcome();
    while(choice != 4) {
        code = new Huffman();
        choice = _menu(src);
        if (choice == 1) {
            _encode(code, src, 0);
        } else if(choice == 2) {
            _encode(code, src, 1);
        } else if(choice == 3) {
            _de_compress(code, src);
        }
        if(choice != 4) {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
            std::cout << "press \"Enter\" to continue >> ";
            getchar();
            std::cout << endl;
        }
        delete code;
    }
}


/*************************************************************************
* private function
*************************************************************************/

// 欢迎！
void _welcome()
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
    cout << "+------------------------------------------------------------------------------------+" << endl;
    cout << "|                        A Simple Huffman encoding Program                           |" << endl;
    cout << "|               It can encode **ANY** file up to 4GB! Just enjoy it!!                |" << endl;
}

// 显示菜单，获取用户输入并判断输入是否有效
unsigned _menu(string &input)
{
    string temp;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
    cout << "+------------------------------------------------------------------------------------+" << endl;
    cout << "|                                                                                    |" << endl;
    cout << "|         +--MENU------------------------------------------+                         |" << endl;
    cout << "|         |                                                |                         |" << endl;
    cout << "|         |      1. Encode string.                         |                         |" << endl;
    cout << "|         |      2. Encode file.                           |                         |" << endl;
    cout << "|         |      3. Uncompress file.                       |                         |" << endl;
    cout << "|         |      4. Exit.                                  |                         |" << endl;
    cout << "|         +------------------------------------------------+                         |" << endl;
    cout << "|                                                                                    |" << endl;
    cout << "+------------------------------------------------------------------------------------+" << endl;
    cout << "Please make a choice(1, 2, 3 or 4): ";
    while (1) {
        getline(cin, temp);
        if (temp[0] == '1') {
            cout << "Please input characters: ";
            break;
        } else if (temp[0] == '2') {
            cout << "Please input the file's path: ";
            break;
        } else if (temp[0] == '3') {
            cout << "Please input the name of Compressed file: ";
            break;
        } else if (temp[0] == '4') {
            return 4;
        } else {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
            cout << "Invalid input!!! ";
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
            cout << "please input \"1\", \"2\", \"3\" or \"4\": ";
        }
    }
    while(1) {
        getline(cin, input);
        if(!input.empty()) {
            break;
        } else {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
            cout << "Empty input!!! ";
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
            cout << "please try again: ";
        }
    }

    return unsigned(temp[0] - '1' + 1);
}

// 压缩字符串或文件
void _compress(Huffman *code, std::string src, bool FileOrStr)
{
    std::string temp;
    std::string dst;
    Huffman::huffman_err op_state;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
    std::cout << "Would you like to compress the current source?(\"y\" or \"n\" ?) ";

    while(1) {
        getline(cin, temp);
        if (temp[0] == 'y' || temp[0] == 'Y') {
            std::cout << "Please input the name of compressed file: ";
            break;
        } else if (temp[0] == 'n' || temp[0] == 'N') {
            return;
        } else {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
            cout << "Invalid input!!! ";
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
            cout << "please input \"y/Y\" or \"n/N\": ";
        }
    }

    while(1) {
        getline(cin, dst);
        if(!dst.empty()) {
            break;
        } else {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
            cout << "Empty input!!! ";
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
            cout << "please try again: ";
        }
    }

    op_state = FileOrStr ? code->compress(src.c_str(), dst.c_str()) : code->compress(src, dst.c_str());

    switch (op_state)
    {
    case Huffman::DST_ERR:
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
        std::cout << "ERROR!!! ";
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
        std::cout << "failed to creat \"" << dst << "\"! Return..." << endl;
    case Huffman::HUFFMAN_OK:
        std::cout << "Compress completed! Return..." << endl;
        break;
    }
}

// 解压文件
void _de_compress(Huffman *code, std::string &src)
{
    std::string dst;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
    std::cout << "Please input the name of UNCompressed file: ";

    while(1) {
        getline(cin, dst);
        if(!dst.empty()) {
            break;
        } else {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
            cout << "Empty input!!! ";
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
            cout << "please try again: ";
        }
    }

    switch (code->decompress(src.c_str(), dst.c_str()))
    {
    case Huffman::SOURCE_ERR:
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
        std::cout << "ERROR!!! ";
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
        std::cout << "failed to open \"" << src << "\"! Return..." << endl;
        break;
    case Huffman::DST_ERR:
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
        std::cout << "ERROR!!! ";
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
        std::cout << "failed to creat \"" << dst << "\"! Return..." << endl;
    case Huffman::HUFFMAN_OK:
        std::cout << "Decompress completed! Return..." << endl;
        break;
    }
}

// 为文件或字符串编码
void _encode(Huffman *code, std::string src, bool FileOrStr)
{
    Huffman::huffman_err op_state = FileOrStr ? code->Encode(src.c_str()) : code->Encode(src);
    switch (op_state)
    {
    case Huffman::FILE_OPEN_ERR:
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
        std::cout << "ERROR!!! ";
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
        std::cout << "failed to open \"" << src << "\"! Return..." << endl;
        break;
    case Huffman::SOURCE_ERR:
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
        std::cout << "ERROR!!! ";
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
        std::cout << "We cannot encode the source that has only one kind of symbol!! Return..." << endl;
        break;
    case Huffman::HUFFMAN_OK:
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
        code->ShowResult();
        std::cout << endl;
        _compress(code, src, FileOrStr);
        break;
    }
}
