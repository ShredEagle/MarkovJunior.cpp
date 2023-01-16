#pragma once

#include "Commons.h"
#include "ImageHelpers.h"
#include "Interpreter.h"
#include "Node.h"

#include <cassert>
#include <cmath>
#include <functional>
#include <math/Vector.h>
#include <pugixml.hpp>

namespace ad {
namespace markovjunior {

class ConvChain : public Node
{
  public:

    ConvChain(const pugi::xml_node & aXmlNode,
              const SymmetryGroup & aSymmetryGroup,
              Interpreter * aInterpreter,
              Grid * aGrid);

    void toggle(std::vector<unsigned char> & aState, int i)
    {
        aState.at(i) = aState.at(i) == mC0 ? mC1 : mC0;
    }

    void reset() override
    {
        mCounter = 0;

        std::fill(mSubstrate.begin(), mSubstrate.end(), false);
    };

    bool run() override;

    void debugRender() override;

    int mN;
    double mTemperature;
    std::vector<double> mWeights;
    unsigned char mC0;
    unsigned char mC1;
    std::vector<bool> mSubstrate;
    unsigned char mSubstrateColor;
    int mCounter;
    int mSteps;
    SamplePattern<bool> mSample;
    math::Size<3, int> mSize;
};

} // namespace markovjunior
} // namespace ad
