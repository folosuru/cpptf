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

template<typename T> 
inline void isTrue(const test_name&, T);

template<typename T> 
inline void isFalse(const test_name&, T);

inline void allTrue(const std::vector<std::pair<test_name,bool>>&&);

class Section {
public:
    static Section& create(const std::string& name);

    template<typename T,typename U> inline void isSame(const test_name&,T ,U);

    inline void except_any(const test_name&, const std::function<void()>& func);

    template<typename T> inline void isTrue(const test_name&, T);

    template<typename T> inline void isFalse(const test_name&, T);

    inline void allTrue(const std::vector<std::pair<test_name,bool>>&&);

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

namespace impl {
namespace util {
inline std::string section_name_colum(std::string str);
inline std::string section_name_colum_center(const std::string& str);
inline std::string status_colum(std::string str);
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
        std::cout << util::status_colum("stats") << util::section_name_colum_center("section / failed") << "passed" << std::endl;
        std::cout << "==============================================" << std::endl;
        size_t check_passed = 0;
        size_t check_total = 0;
        for (const auto& i : sections) {
            if (i.second->cases.empty()) continue;
            auto [pass, total] = i.second->print_result();
            check_passed += pass;
            check_total += total;
        }
        std::cout << "==============================================" << std::endl;
        if (check_passed == check_total) {
            std::cout << util::status_colum(" [o]") << util::section_name_colum("total") << " [" << check_passed << "/" << check_total << "]" << std::endl;
            return true;
        } else {
            std::cout << util::status_colum(" [x]") << util::section_name_colum("total") << " [" << check_passed << "/" << check_total << "]" << std::endl;
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
private:
    inline static std::shared_ptr<General> instance;
    General() = default;
    std::shared_ptr<Section> now_section = std::shared_ptr<Section>(new Section("general"));
    std::unordered_map<section_name, std::shared_ptr<Section>> sections = {{now_section->name ,now_section}};
    friend Section;
    void addSection(Section* sec) {
        std::shared_ptr<Section> new_ptr(sec);
        sections.insert({new_ptr->name ,new_ptr});
        now_section = new_ptr;
    }
};

}
namespace util {
std::string section_name_colum(std::string str) {
    str.resize(32, ' ');
    return str;
}
std::string section_name_colum_center(const std::string& str) {
    size_t padding = (32 - str.size())/2;
    std::string padding_str;
    padding_str.resize(padding,' ');
    if (str.size() % 2 == 1) {
        return padding_str + str + padding_str + " ";
    }
    return padding_str + str + padding_str;
}
std::string status_colum(std::string str) {
    str.resize(6, ' ');
    return str;
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
    if (count == cases.size()) {
        std::cout << impl::util::status_colum(" [o]") << impl::util::section_name_colum(this->name)
            << " [" << count << "/" << cases.size() << "]" << std::endl;
    } else {
        std::cout << impl::util::status_colum(" [x]") << impl::util::section_name_colum(this->name)
            << " [" << count << "/" << cases.size() << "]" << std::endl;
    }
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
void Section::isSame(const test_name&, T p1, U p2) {
    if (impl::util::run_or_return(p1) == impl::util::run_or_return(p2)) {
        add(std::make_shared<impl::data::test_case>(name,true));
    } else {
        add(std::make_shared<impl::data::test_case>(name,false));
    }
}


void except_any(const test_name& name, const std::function<void()>& func) {
    impl::data::General::getInstance()->getNowSection()->except_any(name,func);
}
void Section::except_any(const test_name& name, const std::function<void()>& func) {
    try {
        func();
    } catch (...) {
        add(std::make_shared<impl::data::test_case>(name,true));
        return;
    }
    add(std::make_shared<impl::data::test_case>(name,false));
}

template<typename T> void isTrue(const test_name& name, T obj) {
    impl::data::General::getInstance()->getNowSection()->isTrue(name, obj);
}
template<typename T> void Section::isTrue(const test_name& name, T obj) {
    if (impl::util::run_or_return(obj)) {
        add(std::make_shared<impl::data::test_case>(name,true));
        return;
    }
    add(std::make_shared<impl::data::test_case>(name,false));
}
template<typename T> void isFalse(const test_name& name, T obj) {
    impl::data::General::getInstance()->getNowSection()->isFalse(name, obj);
}
template<typename T> void Section::isFalse(const test_name& name, T obj) {
    if (!impl::util::run_or_return(obj)) {
    add(std::make_shared<impl::data::test_case>(name,true));
        return;
    }
    add(std::make_shared<impl::data::test_case>(name,false));
}

void allTrue(const std::vector<std::pair<test_name,bool>>&& list) {
    for (const auto& i : list) {
        isTrue(i.first,i.second);
    }
}
}  // cpptf
#endif //CPPTF_CPPTF_HPP_
