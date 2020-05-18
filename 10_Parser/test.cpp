#include <functional>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>

#include "./memcheck_crt.h"
#include "./lexer.hpp"
#include "./parser.hpp"
#include "./interpreter.hpp"

using Scope = std::unordered_map<std::string, int>;

struct TestData {
    std::string data;
    Scope answers;
};

std::vector<TestData> test_cases({
    {
        R"(
        BEGIN
            BEGIN
                number := 2;
                a := number;
                b := 10 * a + 10 * number / 4;
                c := a - - b;
            END;
            x := 11;
        END.
        )",
        {   
            { "number", 2 }, 
            { "a", 2 }, 
            { "b", 25 },
            { "c", 27 },
            { "x", 11 },
        }
    }
});

auto get_scope(std::string& data) {
    std::stringstream stream(data);
    Lexer lexer(stream);
    Parser parser(lexer);
    Interpreter interpreter(parser);
    interpreter.interprete();
    return interpreter.scope;
}

bool check_scope(TestData& test_data) {
    auto scope = get_scope(test_data.data);
    for ( auto const& [key, val] : test_data.answers) {
        if (scope[key] != val) {
            std::cout << "Error! \"" << key << "\" = " << scope[key] << ", expected " << val << "\n";
            return false;
        }
    }
    return true;
}

using TestFunc = std::function<bool()>;
using TestGetter = std::function<std::vector<TestFunc>()>;


std::vector<TestFunc> build_test_functions() {
    std::vector<TestFunc> result;
    for (auto& test_data : test_cases) {
        result.push_back([&test_data] { return check_scope(test_data); });
    }
    return result;
}


std::vector<TestFunc> get_tests() {
    return build_test_functions();
}

int run_tests(const TestGetter& tests_getter) {
    std::vector<TestFunc> tests = tests_getter();
    std::size_t tests_count = tests.size();
    bool failed = false;
    bool test_result = false;
    std::string error_msg;
    for (std::size_t i = 0; i < tests_count; ++i) {
        std::cout << "Running test " << i + 1 << "/" << tests_count << "... ";
        error_msg = "";
        try {
            test_result = tests[i]();
        }
        catch (std::exception & e) {
            error_msg = e.what();
            test_result = false;
        }
        failed = failed || !test_result;
        if (test_result) {
            std::cout << " OK";
        }
        else {
            std::cerr << std::endl << "Failed!" << std::endl;
            if (error_msg != "") {
                std::cerr << error_msg;
            }
        }
        std::cout << std::endl;
    }
    if (!failed) {
        std::cout << "All tests passed" << std::endl;
    }
    else {
        std::cout << "Some tests failed, check log for more details" << std::endl;
    }
    return 0;
}

int main() {
    //ENABLE_CRT;
    run_tests(&get_tests);
}