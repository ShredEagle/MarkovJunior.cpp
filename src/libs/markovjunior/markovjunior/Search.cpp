
#include "Search.h"

#include "Constants.h"
#include "Observation.h"
#include "Rule.h"

#include <iostream>
#include <list>
#include <queue>
#include <random>
#include <vector>

namespace ad {
namespace markovjunior {

void enumerateSolution(std::vector<std::vector<unsigned char>> & aChildren,
        std::vector<std::tuple<const Rule *, int>> & aSolution,
        const std::vector<std::tuple<const Rule *, int>> & aTiles,
        std::vector<int> & aAmounts,
        std::vector<bool> & aMask,
        const std::vector<unsigned char> & aState,
        int width)
{
    unsigned int maxIndex = std::distance(aAmounts.begin(), std::max_element(aAmounts.begin(), aAmounts.end()));

    math::Position<3, int> maxIndexPos{maxIndex % width, maxIndex / width, 0};

    if (maxIndex == aAmounts.size())
    {
        aChildren.emplace_back(applyToState(aState, aSolution, width));
        return;
    }

    std::vector<std::tuple<const Rule *, int>> cover;

    for (unsigned int i = 0; i < aTiles.size(); i++)
    {
        auto [rule, ruleIndex] = aTiles.at(i);
        if (aMask.at(i) && isInsideRule(maxIndexPos, rule, {ruleIndex % width, ruleIndex / width, 0}))
        {
            cover.emplace_back(rule, ruleIndex);
        }
    }

    for(auto [rule, ruleIndex] : cover)
    {
        aSolution.emplace_back(rule, ruleIndex);

        std::vector<int> intersecting;

        for (unsigned int i = 0; i < aTiles.size(); i++)
        {
            if (aMask.at(i))
            {
                auto [otherRule, otherRuleIndex] = aTiles.at(i);

                if (overlap(rule,
                            {ruleIndex % width, ruleIndex / width, 0},
                            otherRule,
                            {otherRuleIndex % width, otherRuleIndex / width, 0}))
                {
                    intersecting.push_back(i);
                }
            }
        }

        for (int i : intersecting)
        {
            setMaskAnIncrement(false, i, aTiles, aAmounts, aMask, width);
        }

        enumerateSolution(aChildren, aSolution, aTiles, aAmounts, aMask, aState, width);

        for (int i : intersecting)
        {
            setMaskAnIncrement(true, i, aTiles, aAmounts, aMask, width);
        }

        aSolution.pop_back();
    }
}

std::vector<std::vector<unsigned char>>
allChildStates(const std::vector<unsigned char> & aState,
               const math::Size<3, int> & aSize,
               const std::vector<Rule> & aRules)
{
    std::vector<std::tuple<const Rule *, int>> tiles;
    std::vector<int> amounts(aState.size(), 0);

    for (unsigned int i = 0; i < aState.size(); i++)
    {
        int x = i % aSize.width();
        int y = i / aSize.width();

        for (const Rule & rule : aRules)
        {
            if (matchRule(rule, {x, y, 0}, aState, aSize))
            {
                tiles.emplace_back(&rule, i);

                for (int y = 0; y < rule.mInputSize.height(); y++)
                {
                    for (int x = 0; x < rule.mInputSize.width(); x++)
                    {
                        amounts.at(getFlatIndex({x, y, 0}, aSize))++;
                    }
                }
            }
        }
    }

    std::vector<bool> mask(tiles.size(), true);
    std::vector<std::tuple<const Rule *, int>> solution;

    std::vector<std::vector<unsigned char>> result;
    enumerateSolution(result, solution, tiles, amounts, mask, aState, aSize.width());
    
    return result;
}

std::vector<std::vector<unsigned char>>
oneChildStates(const std::vector<unsigned char> & aState,
        math::Size<3, int> aSize,
        const std::vector<Rule> & aRules)
{
    std::vector<std::vector<unsigned char>> result;
    for (const Rule & rule : aRules)
    {
        for (int y = 0; y < aSize.height(); y++)
        {
            for (int x = 0; x < aSize.width(); x++)
            {
                if (matchRule(rule, {x, y, 0}, aState, aSize))
                {
                    result.push_back(applied(rule, {x, y, 0}, aState, aSize.width()));
                }
            }
        }
    }

    return result;
}

void applyRule(const Rule * aRule,
        const math::Position<3, int> & aPos,
        std::vector<unsigned char> & aState,
        int width)
{
    for (int y = 0; y < aRule->mOutputSize.height(); y++)
    {
        for (int x = 0; x < aRule->mOutputSize.width(); x++)
        {
            math::Position<3, int> shiftedPos = aPos + math::Vec<3, int>{x, y, 0};
            aState.at(getFlatIndex(shiftedPos, {width, 1, 1})) = aRule->mOutputs.at(getFlatIndex({x, y, 0}, aRule->mOutputSize));
        }
    }
}

std::vector<unsigned char>
applyToState(const std::vector<unsigned char> & aState,
        const std::vector<std::tuple<const Rule *, int>> & aSolution,
        int width)
{
    std::vector<unsigned char> result = aState;

    for (auto [rule, i] : aSolution)
    {
        applyRule(rule, {i % width, i / width, 0}, result, width);
    }

    return result;
}

bool isInsideRule(const math::Position<3, int> aPos,
        const Rule * aRule,
        const math::Position<3, int> aRulePos)
{
    return (
            aRulePos.x() <= aPos.x() &&
            aPos.x() < aRulePos.x() + aRule->mInputSize.width() &&
            aRulePos.y() <= aPos.y() &&
            aPos.y() < aRulePos.y() + aRule->mInputSize.height()); 
}

bool overlap(const Rule * aRule,
        const math::Position<3, int> aRulePos,
        const Rule * aOtherRule,
        const math::Position<3, int> aOtherRulePos)
{
    for (int y = 0; y < aRule->mInputSize.height(); y++)
    {
        for (int x = 0; x < aRule->mInputSize.width(); x++)
        {
            if (isInsideRule(aRulePos + math::Vec<3, int>{x, y, 0},
                        aOtherRule, aOtherRulePos))
            {
                return true;
            }
        }
    }

    return false;
}

void setMaskAnIncrement(
        bool value,
        int index,
        const std::vector<std::tuple<const Rule *, int>> & aTiles,
        std::vector<int> & aAmounts,
        std::vector<bool> & aMask,
        int width)
{
    aMask.at(index) = value;
    auto [rule, ruleIndex] = aTiles.at(index);
    math::Position<3, int> pos{ruleIndex % width, ruleIndex / width, 0};

    int increment = value ? 1 : -1;

    for (int y = 0; y < rule->mInputSize.height(); y++)
    {
        for (int x = 0; x < rule->mInputSize.width(); x++)
        {
            aAmounts.at(getFlatIndex(pos + math::Vec<3, int>{x, y, 0}, {width, 1, 1})) += increment;
        }
    }
}


bool matchRule(const Rule & aRule,
               const math::Position<3, int> & aPos,
               const std::vector<unsigned char> & aState,
               const math::Size<3, int> & aSize)
{
    if (aPos.x() + aRule.mInputSize.width() > aSize.width() || aPos.y() + aRule.mInputSize.height() > aSize.height())
    {
        return false;
    }

    for (int y = 0; y < aRule.mInputSize.height(); y++)
    {
        for (int x = 0; x < aRule.mInputSize.width(); x++)
        {
            int ruleFlatIndex = getFlatIndex({x, y, 0}, aRule.mInputSize);
            math::Position<3, int> shiftedPos = aPos + math::Vec<3, int>{x, y, 0};
            int stateFlatIndex = getFlatIndex(shiftedPos, aSize);

            if ((aRule.mInputs.at(ruleFlatIndex) & (1 << aState.at(stateFlatIndex))) == 0)
            {
                return false;
            }
        }
    }

    return true;
}

std::vector<unsigned char> applied(const Rule & aRule,
        const math::Position<3, int> & aPos,
        const std::vector<unsigned char> & aState,
        int width)
{
    std::vector<unsigned char> result = aState;

    for (int z = 0; z < aRule.mOutputSize.depth(); z++)
    {
        for (int y = 0; y < aRule.mOutputSize.height(); y++)
        {
            for (int x = 0; x < aRule.mOutputSize.width(); x++)
            {
                unsigned char newValue = aRule.mOutputs.at(getFlatIndex({x, y, z}, aRule.mOutputSize));
                if (newValue != gWildcardShiftValue)
                {
                    result.at(getFlatIndex(aPos + math::Vec<3, int>{x, y, z}, {width, 1, 1})) = newValue;
                }
            }
        }
    }

    return result;
}

std::vector<std::vector<unsigned char>>
runSearch(const std::vector<unsigned char> & aPresent,
          const std::vector<int> & aFuture,
          const std::vector<Rule> & aRules,
          const math::Size<3, int> & aSize,
          int aCharacterSize,
          bool aAll,
          unsigned int aLimit,
          double aDepthCoefficient,
          int aSeed)
{
    std::vector<std::vector<int>> backwardPotentials(
        aCharacterSize, std::vector<int>(aPresent.size(), -1));
    std::vector<std::vector<int>> forwardPotentials(
        aCharacterSize, std::vector<int>(aPresent.size(), -1));

    Observation::computeBackwardPotentials(backwardPotentials, aFuture, aSize, aRules);
    int rootBackwardEstimate =
        Observation::backwardPointwise(backwardPotentials, aPresent);
    Observation::computeForwardPotentials(forwardPotentials, aPresent, aSize, aRules);
    int rootForwardEstimate = Observation::forwardPointwise(forwardPotentials, aFuture);

    if (rootBackwardEstimate < 0 || rootForwardEstimate < 0) {
        std::cout << "HIIIIN HIIIIN WRONG PROBLEM" << std::endl;
    }

    if (rootBackwardEstimate == 0) {
        return {{}};
    }

    std::vector<Board> database;
    database.emplace_back(aPresent, -1, 0, rootBackwardEstimate, rootForwardEstimate);

    std::map<std::vector<unsigned char>, int> visited;

    visited.emplace(aPresent, 0);

    constexpr auto priorityComp = [](const auto & aLhs, const auto & aRhs) {
        return std::get<double>(aLhs) > std::get<double>(aRhs);
    };

    std::priority_queue<std::tuple<int, double>, std::vector<std::tuple<int, double>>,
                        decltype(priorityComp)>
        frontier(priorityComp);

    std::mt19937 random(aSeed);
    frontier.emplace(0, database.at(0).rank(random, aDepthCoefficient));

    int record = rootBackwardEstimate + rootForwardEstimate;

    while (frontier.size() > 0 && (aLimit < 0 || database.size() < aLimit))
    {
        int parentIndex = std::get<int>(frontier.top());
        frontier.pop();

        const Board & parentBoard = database.at(parentIndex);

        auto children = aAll ? allChildStates(parentBoard.mState, aSize, aRules) : oneChildStates(parentBoard.mState, aSize, aRules);

        for (const auto & childState : children)
        {
            bool success = visited.contains(childState);

            if (success)
            {
                int childIndex = visited.at(childState);

                Board & oldBoard = database.at(childIndex);

                if (parentBoard.mDepth + 1 < oldBoard.mDepth)
                {
                    oldBoard.mDepth = parentBoard.mDepth + 1;
                    oldBoard.mParentIndex = parentIndex;

                    if (oldBoard.mBackwardEstimate >= 0 && oldBoard.mForwardEstimate >= 0)
                    {
                        frontier.emplace(childIndex, oldBoard.rank(random, aDepthCoefficient));
                    }
                }
            }
            else
            {
                int childBackwardEstimate = Observation::backwardPointwise(backwardPotentials, childState);
                Observation::computeForwardPotentials(forwardPotentials, childState, aSize, aRules);
                int childForwardEstimate = Observation::forwardPointwise(forwardPotentials, aFuture);


                if (childBackwardEstimate < 0 || childForwardEstimate < 0)
                {
                    continue;
                }


                database.emplace_back(childState, parentIndex, database.at(parentIndex).mDepth + 1, childBackwardEstimate, childForwardEstimate);
                Board & childBoard = database.back();
                int childIndex = database.size() - 1;
                visited.emplace(childBoard.mState, childIndex);
                //printState(std::vector<int>(childBoard.mState.begin(), childBoard.mState.end()), aSize);

                if (childBoard.mForwardEstimate == 0)
                {
                    std::vector<Board> trajectory = Board::trajectory(childIndex, database);
                    std::vector<Board> reverseTrajectory(trajectory.rbegin(), trajectory.rend());

                    std::vector<std::vector<unsigned char>> result;
                    result.reserve(reverseTrajectory.size());

                    for (const auto & board : reverseTrajectory)
                    {
                        result.push_back(board.mState);
                    }

                    return result;
                }
                else
                {
                    if (childBackwardEstimate + childForwardEstimate < record)
                    {
                        record = childBackwardEstimate + childForwardEstimate;
                    }

                    frontier.emplace(childIndex, childBoard.rank(random, aDepthCoefficient));
                }
            }
        }
    }

    return std::vector<std::vector<unsigned char>>{};
};

} // namespace markovjunior
} // namespace ad
