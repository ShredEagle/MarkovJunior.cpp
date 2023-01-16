#pragma once

#include "RuleNode.h"
#include <imgui.h>

namespace ad {
namespace markovjunior {

class OneNode : public RuleNode
{
public:
    OneNode(const pugi::xml_node & aXmlNode, const SymmetryGroup & aParentSymmetry, Interpreter * aInterpreter, Grid * aGrid) :
        RuleNode(aXmlNode, aParentSymmetry, aInterpreter, aGrid)
    {
        mMatchMask = std::vector<std::vector<bool>>(
            mRules.size(), std::vector<bool>(
                mGrid->mState.size(), false));
    }

    void reset() override
    {
        RuleNode::reset();
        if (mMatchCount != 0)
        {
            for (auto & maskLine : mMatchMask)
            {
                std::fill(maskLine.begin(), maskLine.end(), false);
            }

            mMatchCount = 0;
        }
    }

    void apply(Rule rule, math::Position<3, int> rulePos);

    RuleMatch getRandomMatch(std::mt19937 & aRandom);

    bool run() override;

    void debugRender() override { ImGui::Text("one"); }
};

}
}
