#pragma once

#include "Grid.h"

#include <math/Vector.h>

#include <pugixml.hpp>

#include <vector>

namespace ad {
namespace markovjunior {

typedef std::vector<std::vector<math::Position<3, int>>> RuleShift;

class Rule
{
public:
    Rule(
        std::vector<int> aInput,
        math::Size<3, int> aInputSize,
        std::vector<unsigned char> aOutput,
        math::Size<3, int> aOutputSize, int aCount, double aP);
    Rule(const pugi::xml_node & aXmlNode, const Grid & aGridIn, const Grid & aGridOut);
    Rule reflect();
    Rule rotate();
    void setupShifts(int aCount);

    std::tuple<std::vector<unsigned char>, math::Size<3, int>> parsePatternString(const std::string & patternString);

    bool mOriginal = false;
    std::vector<int> mInputs;
    math::Size<3, int> mInputSize;
    std::vector<unsigned char> mOutputs;
    math::Size<3, int> mOutputSize;
    std::vector<unsigned char> mByteInput;
    RuleShift mInputShifts;
    RuleShift mOutputShifts;
    double mP = 1.0;

    bool operator==(const Rule & aRhs) const;
    bool operator!=(const Rule & aRhs) const = default;

    friend std::ostream & operator<<(std::ostream & os, const Rule & aRule);
};

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

}
}
