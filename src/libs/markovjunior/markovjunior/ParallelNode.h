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

    void addMatch(const int ruleIndex, const math::Position<3, int> & aMatchPosition, std::vector<bool> & aMatchMask) override
    {
        Rule rule = mRules.at(ruleIndex);

        if (mInterpreter->mProbabilityDistribution(mInterpreter->mRandom) > rule.mP)
        {
            return;
        }

        mLast.at(ruleIndex) = true;

        for (int z = 0; z < rule.mOutputSize.depth(); z++)
        {
            for (int y = 0; y < rule.mOutputSize.height(); y++)
            {
                for (int x = 0; x < rule.mOutputSize.width(); x++)
                {
                    unsigned char newValue = rule.mOutputs.at(getFlatIndex({x, y, z}, rule.mOutputSize));

                    math::Position<3, int> statePos = aMatchPosition + math::Vec<3, int>{x, y, z};

                    int i = getFlatIndex(statePos, mInterpreter->mGrid.mSize);

                    if (newValue != 0xff && newValue != mInterpreter->mGrid.mState.at(i))
                    {

                        mNewState.at(i) = newValue;
                        mInterpreter->mChanges.push_back(statePos);
                    }
                }
            }
        }

        mMatchCount++;
    }

    bool run() override
    {
        if (!RuleNode::run())
        {
            return false;
        }

        for (int n = mInterpreter->mFirst.at(mInterpreter->mCounter); n < mInterpreter->mChanges.size(); n++)
        {
            int i = getFlatIndex(mInterpreter->mChanges.at(n), mInterpreter->mGrid.mSize);
            mInterpreter->mGrid.mState.at(i) = mNewState.at(i);
        }

        mCounter++;
        return mMatchCount > 0;
    }

    std::vector<unsigned char> mNewState;
};
}
}
