#pragma once

#include "RuleNode.h"
#include "Interpreter.h"
#include <imgui.h>
#include <random>

namespace ad {
namespace markovjunior {
class ParallelNode : public RuleNode
{
public:
    ParallelNode(const pugi::xml_node & aXmlNode, const SymmetryGroup & aParentSymmetry, Interpreter * aInterpreter, Grid * aGrid) :
        RuleNode(aXmlNode, aParentSymmetry, aInterpreter, aGrid),
        mNewState(aInterpreter->mGrid.mState.size(), 0)
    {
        mMatchMask = std::vector<std::vector<bool>>(
            mRules.size(), std::vector<bool>(
                mGrid->mState.size(), false));
    }

    void addMatch(const int ruleIndex, const math::Position<3, int> & aMatchPosition, std::vector<bool> & aMatchMask) override;

    bool run() override;

    void debugRender(int id = 0) override { ImGui::Text("parallel"); }

    std::vector<unsigned char> mNewState;
};
}
}
