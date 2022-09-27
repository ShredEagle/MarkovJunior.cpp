#pragma once

#include <iostream>
#include <vector>
#include <string>

namespace ad {
namespace markovjunior {

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
            unsigned char min = bounds.at(0)[0] - 48; 
            unsigned char max = bounds.at(1)[0] - 48; 

            for (int i = 0; i < max - min + 1; i++)
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

}
}
