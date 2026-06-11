#include "app.h"
#include <iostream>

int main() {
    try {
        DiaryApp app("data");
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "系統錯誤: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
