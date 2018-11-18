#include "huffman_ui.h"

using namespace std;

int main(int argc, char *argv[])
{
    Huffman code;

    if (argc > 1) {
        CommandMode(code, argv);
    } else {
        IndependenceMode(code);
    }

    return 0;
}