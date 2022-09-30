#include "ParallelNode.h"

#include "Constants.h"

namespace ad {
namespace markovjunior {

void ParallelNode::addMatch(const int ruleIndex, const math::Position<3, int> &aMatchPosition, std::vector<bool> &aMatchMask)
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

                int i = getFlatIndex(statePos, mGrid->mSize);

                if (newValue != gWildcardShiftValue && newValue != mGrid->mState.at(i))
                {

                    mNewState.at(i) = newValue;
                    mInterpreter->mChanges.push_back(statePos);
                }
            }
        }
    }

    mMatchCount++;
}

bool ParallelNode::run()
{
    if (!RuleNode::run())
    {
        return false;
    }

    for (int n = mInterpreter->mFirst.at(mInterpreter->mCounter); n < mInterpreter->mChanges.size(); n++)
    {
        int i = getFlatIndex(mInterpreter->mChanges.at(n), mGrid->mSize);
        mGrid->mState.at(i) = mNewState.at(i);
    }

    mCounter++;
    return mMatchCount > 0;
}


}
}
