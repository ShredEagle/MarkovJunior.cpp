#pragma once

#include "RuleNode.h"

namespace ad {
namespace markovjunior {

class OneNode : public RuleNode
{
public:
    OneNode(const pugi::xml_node & aXmlNode, const SymmetryGroup & aParentSymmetry, Interpreter * aInterpreter) :
        RuleNode(aXmlNode, aParentSymmetry, aInterpreter)
    {
        mMatchMask = std::vector<std::vector<bool>>(
            mRules.size(), std::vector<bool>(
                mInterpreter->mGrid.mState.size(), false));
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

    void apply(Rule rule, math::Position<3, int> rulePos)
    {
        std::vector<math::Position<3, int>> & changes = mInterpreter->mChanges;

        for (int dz = 0; dz < rule.mOutputSize.depth(); dz++)
        {
            for (int dy = 0; dy < rule.mOutputSize.height(); dy++)
            {
                for (int dx = 0; dx < rule.mOutputSize.width(); dx++)
                {
                    char newValue = rule.mOutputs.at(getFlatIndex(
                        {dx, dy, dz},
                        rule.mOutputSize
                        ));
                    math::Position<3, int> newValuePos = rulePos + math::Vec<3, int>{dx, dy, dz};
                    int newValueIndex = getFlatIndex(newValuePos, mInterpreter->mGrid.mSize);
                    char oldValue = mInterpreter->mGrid.mState.at(newValueIndex);

                    if (newValue != oldValue)
                    {
                        mInterpreter->mGrid.mState.at(newValueIndex) = newValue;
                        changes.push_back(newValuePos);
                    }
                }
            }
        }
    }

    RuleMatch getRandomMatch(std::mt19937 & aRandom)
    {
        while(mMatchCount > 0)
        {
            int matchIndex = aRandom() % mMatchCount;
            auto [ruleIndex, matchPos] = mMatches.at(matchIndex);

            int flatIndex = getFlatIndex(matchPos, mInterpreter->mGrid.mSize);

            mMatchMask.at(ruleIndex).at(flatIndex) = false;
            mMatches.at(matchIndex) = mMatches[--mMatchCount];

            if (mInterpreter->mGrid.matchPatternAtPosition(mRules.at(ruleIndex), matchPos))
            {
                return {ruleIndex, matchPos};
            }
        }

        return {-1, {-1, -1, -1}};
    }

    bool run() override
    {
        if (!RuleNode::run())
        {
            return false;
        }

        mLastMatchedTurn = mInterpreter->mCounter;

        auto [ruleIndex, matchPos] = getRandomMatch(mInterpreter->mRandom);

        if (ruleIndex == -1)
        {
            return false;
        }

        mLast.at(ruleIndex) = true;
        apply(mRules.at(ruleIndex), matchPos);
        mCounter++;
        return true;
    }
};

}
}
