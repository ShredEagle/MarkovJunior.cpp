#pragma once

#include "RuleNode.h"
#include "Interpreter.h"
#include <random>

namespace ad {
namespace markovjunior {
class ParallelNode : public RuleNode
{
public:
    ParallelNode(const pugi::xml_node & aXmlNode, const SymmetryGroup & aParentSymmetry, Interpreter * aInterpreter) :
        RuleNode(aXmlNode, aParentSymmetry, aInterpreter),
        mNewState(aInterpreter->mGrid.mState.size(), 0)
    {
        mMatchMask = std::vector<std::vector<bool>>(
            mRules.size(), std::vector<bool>(
                mInterpreter->mGrid.mState.size(), false));
    }

    void addMatch(const int ruleIndex, const math::Position<3, int> & aMatchPosition, std::vector<bool> & aMatchMask) override;

    bool run() override;

    std::vector<unsigned char> mNewState;
};
}
}
