#pragma once

#include "markovjunior/RuleNode.h"

namespace ad {
namespace markovjunior {

class AllNode : public RuleNode
{
public:
    AllNode(const pugi::xml_node & aXmlNode, const SymmetryGroup & aParentSymmetry, Interpreter * aInterpreter) :
        RuleNode(aXmlNode, aParentSymmetry, aInterpreter)
    {
        mMatchMask = std::vector<std::vector<bool>>(
            mRules.size(), std::vector<bool>(
                mInterpreter->mGrid.mState.size(), false));
    }

    void fit(int aRuleIndex, const math::Position<3, int> & aPos, std::vector<bool> & aNewState, const math::Size<3, int> & aSize);

    bool run() override;
};

}
}
