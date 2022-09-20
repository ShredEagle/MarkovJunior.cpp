#pragma once

#include "Grid.h"

#include <math/Vector.h>
#include <random>
#include <vector>

namespace ad {
namespace markovjunior {

struct Board
{
    Board(const std::vector<unsigned char> & aState,
          int aParentIndex,
          int aDepth,
          int aBackwardEstimate,
          int aForwardEstimate) :
        mState{aState},
        mParentIndex{aParentIndex},
        mDepth{aDepth},
        mBackwardEstimate{aBackwardEstimate},
        mForwardEstimate{aForwardEstimate}
    {
    }

    double rank(std::mt19937 & aRandom, double aDepthCoefficient)
    {
        double result = aDepthCoefficient < 0. ? 1000. - mDepth
                                               : mForwardEstimate + mBackwardEstimate
                                                     + 2. * aDepthCoefficient * mDepth;
        return result + 0.0001 * std::uniform_real_distribution<double>{}(aRandom);
    }

    static std::vector<Board> trajectory(int index, const std::vector<Board> & aDatabase)
    {
        std::vector<Board> result;
        for (Board board = aDatabase.at(index); board.mParentIndex >= 0;
             board = aDatabase.at(board.mParentIndex)) {
            result.push_back(board);
        }
        return result;
    }

    std::vector<unsigned char> mState;
    int mParentIndex;
    int mDepth;
    int mBackwardEstimate;
    int mForwardEstimate;
};

std::vector<std::vector<unsigned char>>
runSearch(const std::vector<unsigned char> & aPresent,
          const std::vector<int> & aFuture,
          const std::vector<Rule> & aRules,
          const math::Size<3, int> & aSize,
          int aCharacterSize,
          bool aAll,
          int aLimit,
          double aDepthCoefficient,
          int aSeed);

std::vector<std::vector<unsigned char>>
allChildStates(const std::vector<unsigned char> & aState,
               math::Size<3, int> aSize,
               const std::vector<Rule> & aRules);

std::vector<std::vector<unsigned char>>
oneChildStates(const std::vector<unsigned char> & aState,
        math::Size<3, int> aSize,
        const std::vector<Rule> & aRules);

bool matchRule(const Rule & aRule,
               const math::Position<3, int> & aPos,
               const std::vector<unsigned char> & aState,
               const math::Size<3, int> & aSize);

void enumerateSolution(std::vector<std::vector<unsigned char>> & aChildren,
        const std::vector<std::tuple<const Rule *, int>> & aSolution,
        const std::vector<std::tuple<const Rule *, int>> & aTiles,
        const std::vector<int> & aAmounts,
        const std::vector<bool> & aMask,
        const std::vector<unsigned char> & aState,
        int width);

std::vector<unsigned char>
applyToState(const std::vector<unsigned char> & aState,
        const std::vector<std::tuple<const Rule *, int>> aSolution,
        int width);

void applyRule(const Rule * aRule,
        const math::Position<3, int> & aPos,
        std::vector<unsigned char> & aState,
        int width);

bool isInsideRule(const math::Position<3, int> aPos,
        const Rule * aRule,
        const math::Position<3, int> aRulePos);

bool overlap(const Rule * aRule,
        const math::Position<3, int> aRulePos,
        const Rule * aOtherRule,
        const math::Position<3, int> aOtherRulePos);

void setMaskAnIncrement(
        bool value,
        int index,
        const std::vector<std::tuple<const Rule *, int>> & aTiles,
        std::vector<int> & aAmounts,
        std::vector<bool> & aMask,
        int width);

std::vector<unsigned char> applied(const Rule & aRule,
        const math::Position<3, int> & aPos,
        const std::vector<unsigned char> & aState,
        int width);
} // namespace markovjunior

} // namespace ad
