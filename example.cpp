
#include "Cpptf.hpp"
int main() {
    cpptf::change_section("section1");
    cpptf::isSame("test1", 1, 1);
    cpptf::except_any("except_error",[](){
        auto map = std::unordered_map<int,int>();
        auto t = map.at(5); // it will be std::out_of_range exception.
    });
    cpptf::except_any("except_error2", [](){
        auto map = std::unordered_map<int,int>();
        auto t = map[5]; // it will NOT be std::out_of_range exception.
    });
    cpptf::change_section("section2");
    cpptf::isSame("test2", 1, 1);
    cpptf::isSame("test3", 1, 1);
    cpptf::complete();
    return 0;
}
