#include "OneNode.h"

#include "markovjunior/Interpreter.h"
#include "markovjunior/Rule.h"

#include <cmath>

namespace ad {
namespace markovjunior {

void OneNode::apply(Rule rule, math::Position<3, int> rulePos)
{
    std::vector<math::Position<3, int>> & changes = mInterpreter->mChanges;

    for (int dz = 0; dz < rule.mOutputSize.depth(); dz++)
    {
        for (int dy = 0; dy < rule.mOutputSize.height(); dy++)
        {
            for (int dx = 0; dx < rule.mOutputSize.width(); dx++)
            {
                unsigned char newValue = rule.mOutputs.at(
                    getFlatIndex({dx, dy, dz}, rule.mOutputSize));
                if (newValue != 0xff)
                {
                    math::Position<3, int> newValuePos =
                        rulePos + math::Vec<3, int>{dx, dy, dz};
                    int newValueIndex = getFlatIndex(newValuePos, mGrid->mSize);
                    unsigned char oldValue = mGrid->mState.at(newValueIndex);

                    if (newValue != oldValue)
                    {
                        mGrid->mState.at(newValueIndex) = newValue;
                        changes.push_back(newValuePos);
                    }
                }
            }
        }
    }
}

RuleMatch OneNode::getRandomMatch(std::mt19937 & aRandom)
{
    if (!mPotentials.empty())
    {
        double max = -1000.0;
        int argMax = -1;

        double firstHeuristic = 0.0;
        bool firstHeuristicComputed = false;

        for (unsigned int i = 0; i < mMatchCount; i++)
        {
            auto [ruleIndex, matchPos] = mMatches.at(i);
            int flatIndex = mGrid->getFlatGridIndex(matchPos);

            if (!mGrid->matchPatternAtPosition(mRules.at(ruleIndex), matchPos))
            {
                mMatchMask.at(ruleIndex).at(flatIndex) = false;
                mMatches.at(i) = mMatches.at(--mMatchCount);
                i--;
            }
            else
            {
                std::optional<int> heuristicOpt = Field::deltaPointwise(
                    mGrid->mState, mRules.at(ruleIndex), matchPos, mFields,
                    mPotentials, mGrid->mSize);

                if (!heuristicOpt)
                {
                    continue;
                }

                double heuristicValue = heuristicOpt.value();

                if (!firstHeuristicComputed)
                {
                    firstHeuristic = heuristicValue;
                    firstHeuristicComputed = true;
                }

                double u = mInterpreter->mProbabilityDistribution(aRandom);
                double key = mTemperature > 0. ? std::pow(
                                 u, std::exp((heuristicValue - firstHeuristic)
                                             / mTemperature))
                                               : -heuristicValue + 0.001 * u;

                if (key > max)
                {
                    max = key;
                    argMax = i;
                }
            }
        }

        return argMax >= 0 ? mMatches.at(argMax) : RuleMatch{-1, {-1, -1, -1}};
    }
    else
    {
        while (mMatchCount > 0)
        {
            int matchIndex = aRandom() % mMatchCount;
            auto [ruleIndex, matchPos] = mMatches.at(matchIndex);

            int flatIndex = mGrid->getFlatGridIndex(matchPos);

            mMatchMask.at(ruleIndex).at(flatIndex) = false;
            mMatches.at(matchIndex) = mMatches[--mMatchCount];

            if (mGrid->matchPatternAtPosition(mRules.at(ruleIndex), matchPos))
            {
                return {ruleIndex, matchPos};
            }
        }
    }

    return {-1, {-1, -1, -1}};
}

bool OneNode::run()
{
    mRunning = true;
    if (!RuleNode::run())
    {
        mRunning = false;
        return false;
    }

    mLastMatchedTurn = mInterpreter->mCounter;

    if (mTrajectory.size() > 0)
    {
        if (mCounter >= mTrajectory.size())
        {
            mRunning = false;
            return false;
        }
        mGrid->mState = mTrajectory.at(mCounter++);
        return true;
    }

    auto [ruleIndex, matchPos] = getRandomMatch(mInterpreter->mRandom);

    if (ruleIndex == -1)
    {
        mRunning = false;
        return false;
    }

    mLast.at(ruleIndex) = true;

    apply(mRules.at(ruleIndex), matchPos);
    mCounter++;
    return true;
}

} // namespace markovjunior
} // namespace ad
