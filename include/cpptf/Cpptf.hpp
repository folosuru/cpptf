#pragma once
#ifndef CPPTF_CPPTF_HPP_
#define CPPTF_CPPTF_HPP_
#include <string>
#include <functional>
#include <utility>
#include <memory>
#include <algorithm>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <mutex>

#ifdef CPPTF_MACRO_TEST
#define CPPTF_CONTACT(a, b) CPPTF_CONTACT_INLINE(a,b)
#define CPPTF_CONTACT_INLINE(a, b) a ## b
#define CPPTF_TEST(function) \
    namespace cpptf::test { \
    namespace { \
        bool CPPTF_CONTACT(cpptf_testfunc_dummy_, __LINE__) = cpptf::impl::data::General::getInstance()\
                                            ->addTestQueue( __FILE__, __LINE__, function);\
    }\
    }
#else
#define CPPTF_TEST(function)
#endif

namespace cpptf {
namespace impl::data {
class General;
class test_case;
}

typedef std::string section_name;
typedef std::string test_name;
typedef std::vector<std::pair<test_name,bool>> test_list;

////// control //////
inline bool complete();

inline int complete_exitstatus();

inline void change_section(const section_name& name);

////// test case //////

template<typename T,typename U>
inline void isSame(const test_name&,T ,U);

/**
 * test pass when throw exception in func.
 */
inline void except_any(const test_name&, const std::function<void()>& func);

template<class Exception> inline void except(const test_name& name, const std::function<void()>& func);

template<typename T> 
inline void isTrue(const test_name&, T);

template<typename T> 
inline void isFalse(const test_name&, T);

inline void allTrue(const std::vector<std::pair<test_name,bool>>&&);

class Section {
public:
    static Section& create(const std::string& name);

    template<typename T,typename U> inline void isSame(const test_name&,T ,U);

    void no_throw(const test_name&, const std::function<void()>& func);

    template<class Exception> void except(const test_name&, const std::function<void()>& func);

    inline void except_any(const test_name&, const std::function<void()>& func);

    template<typename T> inline void isTrue(const test_name&, T);

    template<typename T> inline void isFalse(const test_name&, T);

    Section(const Section&) = delete;
    Section& operator=(const Section&) = delete;
private:
    explicit Section(std::string name);
    std::string name;
    std::vector<std::shared_ptr<impl::data::test_case>> cases;
    std::pair<size_t, size_t> print_result();
    friend impl::data::General;
    void add(const std::shared_ptr<impl::data::test_case>& c) {
        cases.push_back(c);
    }
    friend impl::data::test_case;
};

class TestPattern {
public:
    explicit TestPattern(Section& section, std::string pattern_name) : section_(section), name(std::move(pattern_name)){};

    template<typename T,typename U> inline void isSame(T t,U u) {
        section_.isSame(getTestName(), t,u);
    }

    void no_throw(const std::function<void()>& func) {
        section_.no_throw(getTestName(), func);
    }
    template<class Exception> void except( const std::function<void()>& func) {
        section_.except<Exception>(getTestName(), func);
    }

    inline void except_any(const std::function<void()>& func) {
        section_.except_any(getTestName(), func);
    }

    template<typename T> inline void isTrue(T t) {
        section_.isTrue(getTestName(), t);
    }

    template<typename T> inline void isFalse(T t) {
        section_.isFalse(getTestName(), t);
    }

private:
    Section& section_;
    std::string name;
    int test_count = 0;
    std::string getTestName() {
        ++test_count;
        return name + " #" + std::to_string(test_count);
    }
};

namespace impl {
namespace util {
constexpr inline std::uint_fast16_t output_status_width = 5;
constexpr inline std::uint_fast16_t output_section_name_width = 32;
constexpr inline std::uint_fast16_t output_passed_count_width = 6;
constexpr inline std::uint_fast16_t output_total_width = output_status_width + output_section_name_width + output_passed_count_width;

inline std::string separator();
inline std::string result(std::string colum, size_t passed, size_t test_count);
inline std::string section_name_colum(std::string str, std::uint_fast16_t width = output_section_name_width);
inline std::string section_name_colum_center(const std::string& str);
inline std::string status_colum(std::string str);
inline std::string center(const std::string& str);
}
////// class //////
namespace data {
inline std::mutex build_mutex;
class test_case {
public:
    explicit test_case(std::string name,bool result) : result(result), name(std::move(name)) {}
    static void build(const std::string& name, bool result);
    [[nodiscard]] bool getTestResult() const {return result;}
private:
    bool result;
    std::string name;
    void printResult(bool last) const {
        if (!last) {
            std::cout << util::status_colum("") << "|- " << name << std::endl;
        } else {
            std::cout << util::status_colum("") << "`- " << name << std::endl;
        }
    }
    friend Section;
};

class General {
public:
    std::shared_ptr<Section> getNowSection() {
        return now_section;
    }
    [[nodiscard]] bool print() const {
        runTestQueue();
        std::cout << util::separator() << "\n";
        std::cout << util::status_colum("stats") << util::section_name_colum_center("section / failed") << "passed" << std::endl;
        std::cout << util::separator() << std::endl;
        size_t check_passed = 0;
        size_t check_total = 0;
        for (const auto& i : sections) {
            if (i.second->cases.empty()) continue;
            auto [pass, total] = i.second->print_result();
            check_passed += pass;
            check_total += total;
        }
        std::cout << util::separator() << std::endl;
        std::cout << util::result("total", check_passed, check_total) << std::endl;
        if (check_passed == check_total) {
            return true;
        } else {
            return false;
        }
    }
    void changeSection(const section_name& name) {
        if (this->sections.find(name) == this->sections.end()) {
            addSection(new Section(name));
        }
        now_section = sections.at(name);
    }
    static std::shared_ptr<General> getInstance() {
        if (!instance) {
            instance = std::shared_ptr<General>(new General());
        }
        return instance;
    }
    bool addTestQueue( const std::string& file, int line, const std::function<void()>& func) {
        testQueue.emplace_back(func, file, line);
        return true;
    }

private:
    inline static std::shared_ptr<General> instance;
    std::vector<std::tuple<std::function<void()>, std::string, int>> testQueue;
    General() = default;
    std::shared_ptr<Section> now_section = std::shared_ptr<Section>(new Section("general"));
    std::unordered_map<section_name, std::shared_ptr<Section>> sections = {{now_section->name ,now_section}};
    friend Section;
    void addSection(Section* sec) {
        std::shared_ptr<Section> new_ptr(sec);
        sections.insert({new_ptr->name ,new_ptr});
        now_section = new_ptr;
    }

    static void runTest(const std::function<void()>& func, const std::string& file, int line) {
        std::cout << "\033[A\033[2Krunning: " << file  << ":" << line << "\n";
        func();
    }
    static void removeTestRunningText() {
        std::cout << "\033[A\033[2K\n";
    }
    void runTestQueue() const {
        std::vector<std::tuple<std::string, int, std::string>> exception_thrown;
        std::cout <<"\n";
        for (const auto& [func, file, line] : testQueue) {
            #ifdef CPPTF_TEST_CATCH
                try {
                    runTest(func, file, line);
                } catch (std::exception& e) {
                    exception_thrown.emplace_back(file, line, e.what());
                }
            #else
                runTest(func, file, line);
            #endif
        }
        removeTestRunningText();
        #ifdef CPPTF_TEST_CATCH
            if (!exception_thrown.empty()) {
                std::cout << util::separator() << "\n"
                << util::center("Unchaught exception is occur!") << "\n"
                << util::separator() << "\n";
                for (const auto& [file, line, what] : exception_thrown) {
                    std::cout << "occur at : " << file << ":" << line << "\n"
                              << " - what(): " << what << "\n";
                }
                std::cout << "\n";
            }
        #endif
    }
};

}
namespace util {
inline std::string text_center(const std::string& item, std::int_fast16_t width) {
    const size_t padding = (width - item.size())/2;
    std::string padding_str;
    padding_str.resize(padding,' ');
    if (width - (padding*2 + item.size()) != 0) {
        return padding_str + item + padding_str + " ";
    }
    return padding_str + item + padding_str;
}

std::string separator() {
    std::string result;
    result.resize(output_status_width + output_section_name_width + output_passed_count_width, '=');
    return result;
}

inline std::string status_icon(bool status) {
    if (status) {
        return text_center("[o]", output_status_width);
    } else {
        return text_center("[x]", output_status_width);
    }
}

inline std::string result(std::string colum, size_t passed, size_t test_count) {
    const bool status = passed==test_count;
    const std::string passed_text = std::string("[") + std::to_string(passed) + "/" + std::to_string(test_count) + "]";
    std::string result = status_colum(status_icon(status));
    if (passed_text.size() > output_passed_count_width) {
        result += section_name_colum(colum, output_section_name_width + output_passed_count_width - passed_text.size());
        result += passed_text;
    } else {
        result += section_name_colum(std::move(colum));
        result.resize(output_status_width + output_section_name_width + output_passed_count_width - passed_text.size(), ' ');
        result += passed_text;
    }
    return result;
}

std::string section_name_colum(std::string str, std::uint_fast16_t width) {
    str.resize(width, ' ');
    return str;
}
std::string section_name_colum_center(const std::string& str) {
    const size_t padding = (output_section_name_width - str.size())/2;
    std::string padding_str;
    padding_str.resize(padding,' ');
    if (str.size() % 2 == 1) {
        return padding_str + str + padding_str + " ";
    }
    return padding_str + str + padding_str;
}
std::string status_colum(std::string str) {
    str.resize(output_status_width, ' ');
    return str;
}

inline std::string center(const std::string& str) {
    const size_t padding = (output_total_width - str.size())/2;
    std::string padding_str;
    padding_str.resize(padding,' ');
    if (str.size() % 2 == 1) {
        return padding_str + str + padding_str + " ";
    }
    return padding_str + str + padding_str;
}

template<class T> constexpr auto run_or_return(T value) -> decltype(auto) {
    if constexpr(std::is_invocable_v<T>) {
        return value();
    } else {
        return value;
    }
}


}
}
inline Section& Section::create(const std::string& name) {
    auto* result = new Section(name);
    impl::data::General::getInstance()->addSection(result);
    return *result;
}

inline Section::Section(std::string name) : name(std::move(name)) {
}

inline std::pair<size_t, size_t> Section::print_result() {
    size_t count = std::count_if(cases.begin(), cases.end(), [](const std::shared_ptr<impl::data::test_case>& case_) {
        return case_->getTestResult();
    });
    std::cout << impl::util::result(this->name, count, cases.size()) << std::endl;
    size_t printed_count = 0;
    for (const auto& i : cases) {
        if (!i->getTestResult()) {
            printed_count++;
            i->printResult(printed_count==(cases.size()-count));
        }
    }
    return std::make_pair(count,cases.size());
}

void change_section(const std::string& name) {
    impl::data::General::getInstance()->changeSection(name);
}

////// function impl //////

bool complete() {
    if (impl::data::General::getInstance()->print()) {
        return true;
    } else {
        return false;
    }
}

int complete_exitstatus() {
    return complete() ? 0 : 1;
}

////// test case //////

template<typename T, typename U>
void isSame(const test_name& name, T p1, U p2) {
    impl::data::General::getInstance()->getNowSection()->isSame(name,p1,p2);
}
template<typename T, typename U>
void Section::isSame(const test_name& test_name_, T p1, U p2) {
    if (impl::util::run_or_return(p1) == impl::util::run_or_return(p2)) {
        add(std::make_shared<impl::data::test_case>(test_name_,true));
    } else {
        add(std::make_shared<impl::data::test_case>(test_name_,false));
    }
}

inline void no_throw(const test_name& name, const std::function<void()>& func) {
    impl::data::General::getInstance()->getNowSection()->no_throw(name,func);
}

inline void Section::no_throw(const test_name& test_name_, const std::function<void()>& func) {
    try {
        func();
    } catch (...) {
        add(std::make_shared<impl::data::test_case>(test_name_,false));
        return;
    }
    add(std::make_shared<impl::data::test_case>(test_name_,true));
}

template<class Exception> inline void except(const test_name& name, const std::function<void()>& func) {
    impl::data::General::getInstance()->getNowSection()->except<Exception>(name,func);
}
template<class Exception> void Section::except(const test_name& test_name_, const std::function<void()>& func) {
    try {
        func();
    } catch (Exception& exception) {
        add(std::make_shared<impl::data::test_case>(test_name_,true));
        return;
    }
    add(std::make_shared<impl::data::test_case>(test_name_,false));
}

void except_any(const test_name& name, const std::function<void()>& func) {
    impl::data::General::getInstance()->getNowSection()->except_any(name,func);
}
void Section::except_any(const test_name& test_name_, const std::function<void()>& func) {
    try {
        func();
    } catch (...) {
        add(std::make_shared<impl::data::test_case>(test_name_,true));
        return;
    }
    add(std::make_shared<impl::data::test_case>(test_name_,false));
}

template<typename T> void isTrue(const test_name& name, T obj) {
    impl::data::General::getInstance()->getNowSection()->isTrue(name, obj);
}
template<typename T> void Section::isTrue(const test_name& test_name_, T obj) {
    if (impl::util::run_or_return(obj)) {
        add(std::make_shared<impl::data::test_case>(test_name_,true));
        return;
    }
    add(std::make_shared<impl::data::test_case>(test_name_,false));
}
template<typename T> void isFalse(const test_name& name, T obj) {
    impl::data::General::getInstance()->getNowSection()->isFalse(name, obj);
}
template<typename T> void Section::isFalse(const test_name& test_name_, T obj) {
    if (!impl::util::run_or_return(obj)) {
        add(std::make_shared<impl::data::test_case>(test_name_,true));
        return;
    }
    add(std::make_shared<impl::data::test_case>(test_name_,false));
}

void allTrue(const std::vector<std::pair<test_name,bool>>&& list) {
    for (const auto& i : list) {
        isTrue(i.first,i.second);
    }
}
}  // cpptf
#endif //CPPTF_CPPTF_HPP_
