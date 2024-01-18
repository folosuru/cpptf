// define this to enable macro test
#define CPPTF_MACRO_TEST
#define CPPTF_TEST_CATCH
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

    // Also, You can use cpptf::Section& to change section.
    cpptf::Section& section = cpptf::Section::create("right functions");
    section.isSame("1+1=2", right::add(1,1), 2);
    section.except_any("out of range access", [](){
        std::unordered_map<int,int>().at(5); // it will be std::out_of_range exception.
    });
    section.isTrue("foo is longer than 2", right::longer_than("foo",2));

    // you can use cpptf::TestPattern to do many test...
    cpptf::Section& section2 = cpptf::Section::create("add function test");
    {
        cpptf::TestPattern right_tests(section2, "right add function test");
        right_tests.isSame( right::add(1,1), 2);
        right_tests.isSame( right::add(1,3), 4);
        right_tests.isSame( right::add(1,-3), -2);
    }
    /*
    {
        cpptf::TestPattern wrong_tests(section2, "wrong add function test");
        wrong_tests.isSame( wrong::add(1,1), 2);
        wrong_tests.isSame( wrong::add(1,3), 4);
        wrong_tests.isSame( wrong::add(1,-3), -2);
    }*/
    return cpptf::complete_exitstatus();
}

// You can use macro to define test in anywhere
// CPPTF_TEST(std::function<void()> function)
CPPTF_TEST([]{
    cpptf::change_section("macro_test");
    cpptf::isFalse("macro_test", false);
})
void macro_test_func() {
    cpptf::isFalse("macro_test_2", false);
}
CPPTF_TEST(macro_test_func)

CPPTF_TEST([]{
    std::vector<int>().at(5);
})

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
