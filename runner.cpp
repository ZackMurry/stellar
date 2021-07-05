#include <iostream>

extern "C" {
    void STELLAR_ENTRY();
    int doubleVal(int val);
}

int main() {
//    STELLAR_ENTRY();
    std::cout << "doubled: " << doubleVal(2) << std::endl;
    return 0;
}
