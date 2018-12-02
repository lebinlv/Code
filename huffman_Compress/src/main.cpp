#include "huffman_ui.h"

using namespace std;

int main(int argc, char *argv[])
{
    if (argc > 1) {
        CommandMode(argv);
    } else {
        IndependenceMode();
    }

    return 0;
}