#pragma once

#include "Grid.h"
#include "Commons.h"

#include <math/Vector.h>

#include <pugixml.hpp>

#include <vector>

namespace ad {
namespace markovjunior {

typedef std::vector<std::vector<math::Vec<3, int>>> RuleShift;

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
}
}
