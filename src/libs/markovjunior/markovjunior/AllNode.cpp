#include "AllNode.h"

namespace ad {
namespace markovjunior
{
void AllNode::fit(int aRuleIndex, const math::Position<3, int> & aPos, std::vector<bool> & aNewStateMask, const math::Size<3, int> & aSize)
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

                if (value != 0xff && aNewStateMask.at(getFlatIndex(statePos, aSize)))
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

                    aNewStateMask.at(i) = true;
                    mGrid->mState.at(i) = newValue;
                    mInterpreter->mChanges.push_back(statePos);
                }
            }
        }
    }
}

bool AllNode::run()
{
    if (!RuleNode::run())
    {
        return false;
    }

    mLastMatchedTurn = mInterpreter->mCounter;

    if (mTrajectory.size() > 0)
    {
        if (mCounter >= mTrajectory.size())
        {
            return false;
        }
        mGrid->mState = mTrajectory.at(mCounter++);
        return true;
    }

    if (mMatchCount == 0)
    {
        return false;
    }

    if (!mPotentials.empty())
    {
        double firstHeuristic = 0.;
        bool firstHeuristicComputed = false;

        std::vector<std::tuple<int, double>> listPotentials;
        for (unsigned int i = 0; i < mMatchCount; i++)
        {
            auto [ruleIndex, matchPos] = mMatches.at(i);
            std::optional<int> heuristicOpt = Field::deltaPointwise(
                    mGrid->mState,
                    mRules.at(ruleIndex),
                    matchPos,
                    mFields,
                    mPotentials,
                    mGrid->mSize);

            if (heuristicOpt)
            {
                double heuristicValue = heuristicOpt.value();

                if (!firstHeuristicComputed)
                {
                    firstHeuristic = heuristicValue;
                    firstHeuristicComputed = true;
                }

                double u = mInterpreter->mProbabilityDistribution(mInterpreter->mRandom);
                double key = mTemperature > 0.
                    ? std::pow(u, std::exp((heuristicValue - firstHeuristic) / mTemperature))
                    : -heuristicValue + 0.001 * u;

                listPotentials.emplace_back(i, key);
            }
        }

        std::sort(listPotentials.begin(), listPotentials.end(), [](
                    const std::tuple<int, double> & a, const std::tuple<int, double> & b
                    ){ return -std::get<1>(a) < -std::get<1>(b);});

        for (auto [matchIndex, key] : listPotentials)
        {
            auto [ruleIndex, matchPos] = mMatches.at(matchIndex);
            mMatchMask.at(ruleIndex).at(mGrid->getFlatGridIndex(matchPos)) = false;

            fit(ruleIndex, matchPos, mGrid->mMask, mGrid->mSize);
        }
    }
    else
    {
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
            mMatchMask.at(ruleIndex).at(getFlatIndex(matchPos, mGrid->mSize)) = false;
            fit(ruleIndex, matchPos, mGrid->mMask, mGrid->mSize);
        }
    }

    for (unsigned int n = mInterpreter->mFirst[mLastMatchedTurn]; n < mInterpreter->mChanges.size(); n++)
    {
        auto changePos = mInterpreter->mChanges.at(n);
        mGrid->mMask.at(getFlatIndex(changePos, mGrid->mSize)) = false;
    }

    mCounter++;
    mMatchCount = 0;

    return true;
}

}
}
