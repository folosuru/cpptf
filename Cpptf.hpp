#pragma once
#ifndef TESTER_CPP_GENERAL_HPP_
#define TESTER_CPP_GENERAL_HPP_
#include <string>
#include <functional>
#include <utility>
#include <memory>
#include <algorithm>
#include <iostream>

namespace cpptf {

typedef std::string section_name;
typedef std::string test_name;
typedef std::vector<std::pair<test_name,bool>> test_list;

////// control //////
void complete();

void change_section(const section_name& name);

////// test case //////

template<typename T,typename U> void isSame(const test_name&,T ,U);

/**
 * test pass when throw exception in func.
 */
void except_any(const test_name&, const std::function<void()>& func);

template<typename T> void isTrue(const test_name&, T);

void allTrue(const std::vector<std::pair<test_name,bool>>&&);


////// class //////
namespace data {
class General;
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
class test_case {
public:
    explicit test_case(std::string name,bool result) : name(std::move(name)), result(result) {}
    static void build(const std::string& name,bool result);
    [[nodiscard]] bool getTestResult() const {return result;}
    void printResult(bool last) const {
        if (!last) {
            std::cout << status_colum("") << "|- " << name << std::endl;
        } else {
            std::cout << status_colum("") << "`- " << name << std::endl;
        }
    }

private:
    bool result;
    std::string name;
};


class Section {
public:
    explicit Section(std::string name) : name(std::move(name)) {}

    void add(const std::shared_ptr<test_case>& c) {
        cases.push_back(c);
    }

    std::pair<size_t, size_t> print_result() {
        size_t count = std::count_if(cases.begin(), cases.end(), [](const std::shared_ptr<test_case>& case_) {
            return case_->getTestResult();
        });

        if (count == cases.size()) {
            std::cout << status_colum(" [o]") << section_name_colum(this->name) << " [" << count << "/" << cases.size() << "]" << std::endl;
        } else {
            std::cout << status_colum(" [x]") << section_name_colum(this->name) << " [" << count << "/" << cases.size() << "]" << std::endl;
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

private:
    std::string name;

    std::vector<std::shared_ptr<test_case>> cases;
};

class General {
public:
    std::shared_ptr<Section> getNowSection() {
        if (section_index.find(now_section) == section_index.end()) {
            section_index[now_section] = sections.size();
            sections.push_back(std::make_shared<Section>(now_section));
        }
        return sections[section_index[now_section]];
    }
    bool print() {
        std::cout << status_colum("stats") << section_name_colum_center("section / failed") << "passed" << std::endl;
        std::cout << "==============================================" << std::endl;
        size_t check_passed = 0;
        size_t check_total = 0;
        for (const auto& i : sections) {
            auto result = i->print_result();
            check_passed += result.first;
            check_total += result.second;
        }
        std::cout << "==============================================" << std::endl;
        if (check_passed == check_total) {
            std::cout << status_colum(" [o]") << section_name_colum("total") << " [" << check_passed << "/" << check_total << "]" << std::endl;
            return true;
        } else {
            std::cout << status_colum(" [x]") << section_name_colum("total") << " [" << check_passed << "/" << check_total << "]" << std::endl;
            return false;
        }
    }
    void changeSection(const section_name& name) {
        if (this->section_index.find(name) == this->section_index.end()) {
            section_index[name] = sections.size();
            sections.push_back(std::make_shared<Section>(name));
        }
        now_section = name;
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

    std::unordered_map<section_name , size_t> section_index;

    std::vector<std::shared_ptr<Section>> sections;

    section_name now_section = "general";
};

}
void change_section(const std::string& name) {
    data::General::getInstance()->changeSection(name);
}
void data::test_case::build(const std::string& name, bool result) {
    General::getInstance()->getNowSection()->add(std::make_shared<test_case>(name,result));
}

////// function impl //////

void complete() {
    if (data::General::getInstance()->print()) {
        return;
    } else {
        throw std::exception("");
    }
}

////// test case //////

template<typename T, typename U>
void isSame(const test_name& name, T p1, U p2) {
    if (p1 == p2) {
        data::test_case::build(name,true);
    } else {
        data::test_case::build(name, false);
    }
}

void except_any(const test_name& name, const std::function<void()>& func) {
    try {
        func();
    } catch (...) {
        data::test_case::build(name, true);
        return;
    }
    data::test_case::build(name, false);
}

template<typename T> void isTrue(const test_name& name, T obj) {
    if (obj) {
        data::test_case::build(name, true);
        return;
    }
    data::test_case::build(name, false);
}

void allTrue(const std::vector<std::pair<test_name,bool>>&& list) {
    for (const auto& i : list) {
        isTrue(i.first,i.second);
    }
}

}  // tester_cpp
#endif //TESTER_CPP_GENERAL_HPP_
