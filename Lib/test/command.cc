#include "../CommandLine.h"
#include <iostream>

int main(int argc, char *argv[]) {

    CommandLine cl(argc, argv);

    cl.addCommand('a', "hello a");
    cl.addCommand('b', "hello b");
    cl.addCommand('c', "hello c", "12");

    cl.parseCommand();

    int a = cl['a'];
    std::string b = cl['b'];
    int c = cl['c'];

    std::cout << "a: " << a << std::endl;
    std::cout << "b: " << b << std::endl;
    std::cout << "c: " << c << std::endl;


    return 0;
}
