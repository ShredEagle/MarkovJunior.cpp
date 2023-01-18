#pragma once

#include <functional>
#include <resource/ResourceLocator.h>

#include <cmath>
#include <filesystem>
#include <iostream>
#include <vector>
#include <string>

namespace ad {
namespace markovjunior {

const resource::ResourceLocator gResourceLocator("/home/franz/gamedev/MarkovJunior.cpp/");

inline std::vector<std::string> splitString(std::string aString, const std::string & aDelimiter)
{
    std::vector<std::string> result;

    std::size_t pos = 0;
    std::string token;
    while((pos = aString.find(aDelimiter)) != std::string::npos)
    {
        token = aString.substr(0, pos);
        result.push_back(token);
        aString.erase(0, pos + aDelimiter.length());
    }

    result.push_back(aString);

    return result;
}

inline std::vector<std::vector<std::string>> splitPatternString(std::string aString)
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

inline std::vector<int> splitIntervals(const std::string & aString)
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

            for (unsigned int i = 0; i < (unsigned int)1 + max - min; i++)
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

inline int modulo(int a, int divisor)
{
    const int result = a % divisor;
    return result >= 0 ? result : result + divisor;
}

template<class T>
std::vector<T> createPattern(std::function<T(int, int)> aFunction, int aN)
{
    std::vector<T> result(aN * aN);
    for (int y = 0; y < aN; y++)
    {
        for (int x = 0; x < aN; x++)
        {
            result.at(x + y * aN) = aFunction(x, y);
        }
    }
    return result;
}

template<class T_sampleType>
struct SamplePattern : public std::vector<T_sampleType>
{
    using std::vector<T_sampleType>::vector;

    SamplePattern<T_sampleType> rotate()
    {
        std::vector<T_sampleType> result = createPattern<T_sampleType>(
            [&](int x, int y)
            {
                return this->at((unsigned int)std::sqrt(this->size()) - 1 - y + x * (unsigned int)std::sqrt(this->size()));
            },
            (int)std::sqrt(this->size()));
        return SamplePattern<T_sampleType>(result.begin(), result.end());
    }
    SamplePattern<T_sampleType> reflect()
    {
        std::vector<T_sampleType> result = createPattern<T_sampleType>(
            [&](int x, int y)
            {
                return this->at((unsigned int)std::sqrt(this->size()) - 1 - x + y * (unsigned int)std::sqrt(this->size()));
            },
            (int)std::sqrt(this->size()));
        return SamplePattern<T_sampleType>(result.begin(), result.end());
    }
    int index()
    {
        int result = 0;
        int power = 1;

        for (unsigned int i = 0; i < this->size(); i++, power *= 2)
        {
            if (this->at(i))
            {
                result += power;
            }
        }

        return result;
    }
};

}
}
