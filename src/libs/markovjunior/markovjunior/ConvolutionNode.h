#pragma once

#include "Node.h"
#include "SymmetryUtils.h"

#include <algorithm>
#include <array>
#include <imgui.h>
#include <map>
#include <string>                        // for string, allocator, operator<=>
#include <vector>                        // for vector

namespace pugi {
    class xml_node;
}

namespace ad {
namespace markovjunior {

class Grid;
class Interpreter;

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
    ConvolutionNode(const pugi::xml_node & aXmlNode, const SymmetryGroup & aParentSymmetryGroup, Interpreter * aInterpreter, Grid * aGrid);

    bool run() override;

    void reset() override
    {
        mCounter = 0;
    }

    void debugRender(int id = 0) override { ImGui::Text("ConvolutionNode"); }

    std::vector<ConvolutionRule> mRules;
    std::vector<int> mKernel;
    bool mPeriodic;
    int mCounter;
    int mSteps = 0;
    std::vector<std::vector<int>> mSumField;
};


}
}
