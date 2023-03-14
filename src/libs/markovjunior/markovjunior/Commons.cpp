#include "Commons.h"

namespace ad {
namespace markovjunior {

std::vector<std::string> splitString(std::string aString,
                                     const std::string & aDelimiter)
{
    std::vector<std::string> result;

    std::size_t pos = 0;
    std::string token;
    while ((pos = aString.find(aDelimiter)) != std::string::npos)
    {
        token = aString.substr(0, pos);
        result.push_back(token);
        aString.erase(0, pos + aDelimiter.length());
    }

    result.push_back(aString);

    return result;
}

std::vector<std::vector<std::string>> splitPatternString(std::string aString)
{
    std::vector<std::vector<std::string>> result;
    std::vector<std::string> zPlanes = splitString(std::move(aString), " ");
    result.reserve(zPlanes.size());

    for (const auto & zPlane : zPlanes)
    {
        result.push_back(splitString(zPlane, "/"));
    }

    return result;
}

std::vector<int> splitIntervals(const std::string & aString)
{
    std::vector<int> result;
    std::vector<std::string> intervals = splitString(aString, ",");

    for (const std::string & interval : intervals)
    {
        if (interval.find('.') != std::string::npos)
        {
            std::vector<std::string> bounds = splitString(interval, "..");
            unsigned char min = std::stoi(bounds.at(0));
            unsigned char max = std::stoi(bounds.at(1));

            for (unsigned int i = 0; i < (unsigned int) 1 + max - min; i++)
            {
                result.push_back(min + i);
            }
        }
        else
        {
            result.push_back(interval[0]);
        }
    }

    return result;
}

int modulo(int a, int divisor)
{
    const int result = a % divisor;
    return result >= 0 ? result : result + divisor;
}

std::vector<std::byte>
createPattern(std::function<std::byte(int, int)> aFunction, int aN)
{
    std::vector<std::byte> result(aN * aN);
    for (int y = 0; y < aN; y++)
    {
        for (int x = 0; x < aN; x++)
        {
            result.at(x + y * aN) = aFunction(x, y);
        }
    }
    return result;
}

SamplePattern SamplePattern::rotate()
{
    std::vector result = createPattern(
        [&](int x, int y) {
            return this->at((unsigned int) std::sqrt(this->size()) - 1 - y
                            + x * (unsigned int) std::sqrt(this->size()));
        },
        (int) std::sqrt(this->size()));
    return SamplePattern(result.begin(), result.end());
}
SamplePattern SamplePattern::reflect()
{
    std::vector<std::byte> result = createPattern(
        [&](int x, int y) {
            return this->at((unsigned int) std::sqrt(this->size()) - 1 - x
                            + y * (unsigned int) std::sqrt(this->size()));
        },
        (int) std::sqrt(this->size()));
    return SamplePattern(result.begin(), result.end());
}
int SamplePattern::index()
{
    int result = 0;
    int power = 1;

    for (unsigned int i = 0; i < this->size(); i++, power *= 2)
    {
        if (static_cast<bool>(this->at(i)))
        {
            result += power;
        }
    }

    return result;
}
} // namespace markovjunior
} // namespace ad
