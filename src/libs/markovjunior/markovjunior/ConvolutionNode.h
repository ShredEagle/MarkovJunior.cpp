#pragma once

#include "markovjunior/Grid.h"
#include "markovjunior/Interpreter.h"
#include "markovjunior/Node.h"
#include "markovjunior/SymmetryUtils.h"

#include <pugixml.hpp>

namespace ad {
namespace markovjunior {

class ConvolutionRule
{
public:
    unsigned char mInput;
    unsigned char mOutput;
    std::vector<unsigned char> mValues;
    std::vector<bool> mSums;
    double mProbability;

    ConvolutionRule(const pugi::xml_node & aXmlNode, const Grid & aGrid);
};

const std::map<std::string, std::array<int, 27>> gTwoDKernels = {
    {"VonNeumann", {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0 , 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {"Moore", {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}}
};

const std::map<std::string, std::array<int, 27>> gThreeDKernels = {
    {"VonNeumann", {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0}},
    {"NoCorners", {0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0}}
};

class ConvolutionNode : public Node
{
public:
    ConvolutionNode(const pugi::xml_node & aXmlNode, const SymmetryGroup & aParentSymmetryGroup, Interpreter * aInterpreter);

    bool run() override;

    void reset() override
    {
        mCounter = 0;
    }

    std::vector<ConvolutionRule> mRules;
    std::vector<int> mKernel;
    bool mPeriodic;
    int mCounter;
    int mSteps = 0;
    std::vector<std::vector<int>> mSumField;
};


}
}
