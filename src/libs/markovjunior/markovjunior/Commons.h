#pragma once

#include <functional>

#include <cmath>
#include <vector>
#include <string>

namespace ad {
namespace markovjunior {

std::vector<std::string> splitString(std::string aString, const std::string & aDelimiter);
std::vector<std::vector<std::string>> splitPatternString(std::string aString);
std::vector<int> splitIntervals(const std::string & aString);
int modulo(int a, int divisor);

std::vector<std::byte> createPattern(std::function<std::byte(int, int)> aFunction, int aN);

struct SamplePattern : public std::vector<std::byte>
{
    using std::vector<std::byte>::vector;

    SamplePattern rotate();
    SamplePattern reflect();
    int index();
};

}
}
