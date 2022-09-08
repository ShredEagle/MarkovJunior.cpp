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

    void fit(int aRuleIndex, const math::Position<3, int> & aPos, std::vector<bool> & aNewState, const math::Size<3, int> & aSize)
    {
        Rule rule = mRules.at(aRuleIndex);

        for (int z = 0; z < rule.mOutputSize.depth(); z++)
        {
            for (int y = 0; y < rule.mOutputSize.height(); y++)
            {
                for (int x = 0; x < rule.mOutputSize.width(); x++)
                {
                    unsigned char value = rule.mOutputs.at(getFlatIndex({x, y, z}, rule.mOutputSize));

                    math::Position<3, int> statePos = aPos + math::Vec<3, int>{x, y, z};

                    if (value != 0xff && aNewState.at(getFlatIndex(statePos, aSize)))
                    {
                        return;
                    }
                }
            }
        }

        mLast.at(aRuleIndex) = true;

        for (int z = 0; z < rule.mOutputSize.depth(); z++)
        {
            for (int y = 0; y < rule.mOutputSize.height(); y++)
            {
                for (int x = 0; x < rule.mOutputSize.width(); x++)
                {
                    unsigned char newValue = rule.mOutputs.at(getFlatIndex({x, y, z}, rule.mOutputSize));

                    if (newValue != 0xff)
                    {
                        math::Position<3, int> statePos = aPos + math::Vec<3, int>{x, y, z};

                        int i = getFlatIndex(statePos, aSize);

                        aNewState.at(i) = true;
                        mInterpreter->mGrid.mState.at(i) = newValue;
                        mInterpreter->mChanges.push_back(statePos);
                    }
                }
            }
        }
    }

    bool run() override
    {
        if (!RuleNode::run())
        {
            return false;
        }

        mLastMatchedTurn = mInterpreter->mCounter;

        if (mMatchCount == 0)
        {
            return false;
        }

        std::vector<int> shuffle(mMatchCount, 0);

        int i = 0;
        for (int & value : shuffle)
        {
            int j = mInterpreter->mRandom() % (i + 1);
            value = shuffle.at(j);
            shuffle.at(j) = i;
            i++;
        }

        for (int value : shuffle)
        {
            auto [ruleIndex, matchPos] = mMatches.at(value);
            mMatchMask.at(ruleIndex).at(getFlatIndex(matchPos, mInterpreter->mGrid.mSize)) = false;
            fit(ruleIndex, matchPos, mInterpreter->mGrid.mMask, mInterpreter->mGrid.mSize);
        }

        for (int n = mInterpreter->mFirst[mLastMatchedTurn]; n < mInterpreter->mChanges.size(); n++)
        {
            auto changePos = mInterpreter->mChanges.at(n);
            mInterpreter->mGrid.mMask.at(getFlatIndex(changePos, mInterpreter->mGrid.mSize)) = false;
        }

        mCounter++;
        mMatchCount = 0;
        return true;
    }
};

}
}
