#pragma once

#include "markovjunior/RuleNode.h"
#include <imgui.h>

namespace ad {
namespace markovjunior {

class AllNode : public RuleNode
{
public:
    AllNode(const pugi::xml_node & aXmlNode,
            const SymmetryGroup & aParentSymmetry,
            Interpreter * aInterpreter,
            Grid * aGrid) :
        RuleNode(aXmlNode, aParentSymmetry, aInterpreter, aGrid)
    {
        mAllSearch = true;
        mMatchMask = std::vector<std::vector<bool>>(
            mRules.size(), std::vector<bool>(mGrid->mState.size(), false));
    }

    void fit(int aRuleIndex,
             const math::Position<3, int> & aPos,
             std::vector<bool> & aNewState,
             const math::Size<3, int> & aSize);

    bool run() override;

    void debugRender(int id = 0) override {
        ImGui::Text("all");
        ImGui::SameLine();
        RuleNode::debugRender();
    }
};

} // namespace markovjunior
} // namespace ad
