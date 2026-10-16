#pragma once
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#define CHECK(condition) do { if (!(condition)) throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__) + " " #condition); ++checks; } while(false)
#define NEAR(actual, expected, tolerance) CHECK(std::abs((actual) - (expected)) <= (tolerance))
inline int checks = 0;
template<class F> int run(F f) {
    try { f(); std::cout << checks << " checks passed\n"; return 0; }
    catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
