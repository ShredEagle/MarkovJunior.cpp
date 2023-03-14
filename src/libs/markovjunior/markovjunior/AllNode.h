#pragma once


#include "markovjunior/RuleNode.h"
#include "markovjunior/SymmetryUtils.h"
#include <imgui.h>

#include <math/Vector.h>

#include <vector>

namespace pugi { class xml_node; }

namespace ad {
namespace markovjunior {

class Interpreter;
class Grid;

class AllNode : public RuleNode
{
public:
    AllNode(const pugi::xml_node & aXmlNode,
            const SymmetryGroup & aParentSymmetry,
            Interpreter * aInterpreter,
            Grid * aGrid);

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
