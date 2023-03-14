#pragma once

#include "Commons.h"
#include "Node.h"
#include "markovjunior/SymmetryUtils.h"
#include <math/Vector.h>                // for Size
#include <algorithm>
#include <vector>

namespace pugi { class xml_node; }

namespace ad {
namespace markovjunior {

class Grid;
class Interpreter;

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

    void debugRender(int id = 0) override;

    int mN;
    double mTemperature;
    std::vector<double> mWeights;
    unsigned char mC0;
    unsigned char mC1;
    std::vector<bool> mSubstrate;
    unsigned char mSubstrateColor;
    int mCounter;
    int mSteps;
    SamplePattern mSample;
    math::Size<3, int> mSize;
};

} // namespace markovjunior
} // namespace ad
