#include <iostream>
#include <string>
#include <iomanip>
#include <windows.h>

#include "huffman_ui.h"

using namespace std;

void welcome()
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
    cout << "+------------------------------------------------------------------------------------+" << endl;
    cout << "|                        A Simple Huffman encoding Program                           |" << endl;
    cout << "|               It can encode **ANY** file up to 4GB! Just enjoy it!!                |" << endl;
}

unsigned menu(string &input)
{
    string temp;
    bool read_ok = false;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
    cout << "+------------------------------------------------------------------------------------+" << endl;
    cout << "|                                                                                    |" << endl;
    cout << "|         +--MENU------------------------------------------+                         |" << endl;
    cout << "|         |                                                |                         |" << endl;
    cout << "|         |      1. Encode string.                         |                         |" << endl;
    cout << "|         |      2. Encode file.                           |                         |" << endl;
    cout << "|         |      3. Exit.                                  |                         |" << endl;
    cout << "|         +------------------------------------------------+                         |" << endl;
    cout << "|                                                                                    |" << endl;
    cout << "+------------------------------------------------------------------------------------+" << endl;
    cout << "Please make a choice(1, 2 or 3): ";
    while (!read_ok) {
        getline(cin, temp);
        if (temp[0] == '1') {
            cout << "Please input characters: ";
            break;
        } else if (temp[0] == '2') {
            cout << "Please input the file's path: ";
            break;
        } else if (temp[0] == '3') {
            return 3;
        } else {
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
            cout << "Invalid input!!! ";
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
            cout << "please input \"1\", \"2\" or \"3\": ";
        }
    }
    getline(cin, input);
    return unsigned(temp[0] - '1' + 1);
}

void CommandMode(Huffman &code, char *argv[])
{
    string input;

    if(argv[1][1] == 'f') {
        switch (code.Encode(argv[2])) {
        case Huffman::FILE_OPEN_ERR:
            std::cout << "ERROR!!! failed to open \"" << argv[2] << "\"! Back to menu..." << endl;
            break;
        case Huffman::SOURCE_ERR:
            std::cout << "ERROR!!! We cannot encode the source that has only one kind of symbol!!" << endl;
            break;
        case Huffman::HUFFMAN_OK:
            code.ShowResult();
            break;
        }
    } else if(argv[1][1] == 's') {
        input = argv[2];
        if (code.Encode(input) == Huffman::HUFFMAN_OK) {
            code.ShowResult();
        } else {
            std::cout << "ERROR!!! We cannot encode the source that has only one kind of symbol!!" << endl;
        }
    } else {
        std::cout << "Usage: " << argv[0] << " [-?] [-h] [-f xxx] [-s xxx]" << endl;
        std::cout << "    " << left << setw(10) << "-?";
        std::cout << "Display help." << endl;
        std::cout << "    " << left << setw(10) << "-h";
        std::cout << "Display help." << endl;
        std::cout << "    " << left << setw(10) << "-f xxx";
        std::cout << "treat xxx as file path and encode the file." << endl;
        std::cout << "    " << left << setw(10) << "-s xxx";
        std::cout << "treat xxx as string and encode it." << endl;
    }
}

void IndependenceMode(Huffman &code)
{
    string input;
    const char *file_name = NULL;
    unsigned choice = 0;

    welcome();
    while(choice != 3) {
        choice = menu(input);
        if (choice == 2) {
            file_name = input.c_str();
            switch (code.Encode(file_name)) {
            case Huffman::FILE_OPEN_ERR:
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
                std::cout << "ERROR!!! ";
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
                std::cout << "failed to open \"" << file_name << "\"! Back to menu..." << endl;
                break;
            case Huffman::SOURCE_ERR:
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
                std::cout << "ERROR!!! ";
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
                std::cout << "We cannot encode the source that has only one kind of symbol!! Back to menu..." << endl;
                break;
            case Huffman::HUFFMAN_OK:
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
                code.ShowResult();
                break;
            }
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
            std::cout << "press \"Enter\" to continue >> ";
            getchar();
            std::cout << endl;
        } else if(choice == 1) {
            if (code.Encode(input) == Huffman::HUFFMAN_OK) {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
                code.ShowResult();
            } else {
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
                std::cout << "ERROR!!! ";
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
                std::cout << "We cannot encode the source that has only one kind of symbol!! Back to menu..." << endl;
            }
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
            std::cout << "press \"Enter\" to continue >> ";
            getchar();
            std::cout << endl;
        }
    }
}
