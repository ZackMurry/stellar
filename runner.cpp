#include <iostream>

extern "C" {
    void STELLAR_ENTRY();
    int getValue(int);
}

int main() {
    STELLAR_ENTRY();
    std::cout << "Value is " << getValue(1) << std::endl;
    return 0;
}
