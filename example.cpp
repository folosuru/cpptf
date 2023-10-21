#include "include/cpptf/Cpptf.hpp"

namespace wrong {  // all functions has a bug...
int add(int a, int b);
bool longer_than(std::string string, size_t compare);
}

namespace right {
int add(int a, int b);
bool longer_than(std::string string, size_t compare);
}

int main() {
    cpptf::change_section("wrong functions");
    cpptf::isSame("1+1=2", wrong::add(1,1), 2);
    cpptf::except_any("out of range access", [](){
        std::unordered_map<int,int>()[5]; // it will NOT be std::out_of_range exception.
    });
    cpptf::isTrue("foo is longer_than 2", wrong::longer_than("foo",2));

    cpptf::change_section("right functions");
    cpptf::isSame("1+1=2", right::add(1,1), 2);
    cpptf::except_any("out of range access", [](){
        std::unordered_map<int,int>().at(5); // it will be std::out_of_range exception.
    });
    cpptf::isTrue("foo is longer than 2", right::longer_than("foo",2));
    cpptf::complete();
    return 0;
}

int wrong::add(int a, int b) {
    return a + b + 1;  // yes, this is not right.
}
bool wrong::longer_than(std::string string, size_t compare) {
    return string.size() < compare;  // yes. this isn't work, too.
}
int right::add(int a, int b) {
    return a + b ;  // perfect!
}
bool right::longer_than(std::string string, size_t compare) {
    return string.size() > compare;  // you are genius!
}
