#pragma once

#include "markovjunior/Grid.h"
#include "markovjunior/Rule.h"

#include <queue>
#include <string>
namespace ad {
namespace markovjunior {

class Observation
{
  public:
    Observation() = default;

    Observation(const unsigned char aFrom, const std::string & aTo, const Grid & aGrid) :
        mFrom{aGrid.mValues.at(aFrom)}, mTo{aGrid.makeWave(aTo)}
    {
    }

    static bool computeFutureSetPresent(std::vector<int> & aFuture,
                                        std::vector<unsigned char> & aState,
                                        const std::vector<Observation> & aObservations);
    static void computePotentials(std::vector<std::vector<int>> & aPotentials,
                                  const math::Size<3, int> & aSize,
                                  const std::vector<Rule> & aRules,
                                  bool aBackwards);
    static bool forwardMatches(const Rule & aRule,
                               const math::Position<3, int> & aPos,
                               const std::vector<std::vector<int>> & aPotentials,
                               int aThreshold,
                               const math::Size<3, int> & aSize,
                               bool aBackwards);
    static void
    applyForward(const Rule & aRule,
                 const math::Position<3, int> & aPos,
                 std::vector<std::vector<int>> & aPotentials,
                 int aThreshold,
                 const math::Size<3, int> & aSize,
                 std::queue<std::tuple<unsigned char, math::Position<3, int>>> & aQueue,
                 bool aBackwards);

    static void computeForwardPotentials(std::vector<std::vector<int>> & aPotentials,
                                         const std::vector<unsigned char> & mState,
                                         const math::Size<3, int> & aSize,
                                         const std::vector<Rule> & aRules)
    {
        for (auto & potentialField : aPotentials) {
            std::fill(potentialField.begin(), potentialField.end(), -1);
        }

        for (std::size_t i = 0; i != mState.size(); i++) {
            aPotentials.at(mState.at(i)).at(i) = 0;
        }

        computePotentials(aPotentials, aSize, aRules, false);
    }

    static void computeBackwardPotentials(std::vector<std::vector<int>> & aPotentials,
                                          const std::vector<int> & aFuture,
                                          const math::Size<3, int> & aSize,
                                          const std::vector<Rule> & aRules)
    {
        for (std::size_t c = 0; c < aPotentials.size(); c++) {
            auto & potentialField = aPotentials.at(c);
            for (std::size_t i = 0; i < aFuture.size(); i++) {
                potentialField.at(i) = (aFuture.at(i) & (1 << c)) != 0 ? 0 : -1;
            }
        }

        computePotentials(aPotentials, aSize, aRules, true);
    }

    static bool isGoalReached(const std::vector<unsigned char> & aPresent,
                              const std::vector<int> & aFuture)
    {
        for (std::size_t i = 0; i != aPresent.size(); i++) {
            if (((1 << aPresent.at(i)) & aFuture.at(i)) == 0) {
                return false;
            }
        }
        return true;
    }

    static int forwardPointwise(const std::vector<std::vector<int>> & aPotentials,
                                const std::vector<int> & aFuture)
    {
        int sum = 0;

        for (std::size_t i = 0; i != aFuture.size(); i++) {
            int f = aFuture.at(i);
            int min = 1000;
            int argmin = -1;

            for (std::size_t c = 0; c < aPotentials.size(); c++, f >>= 1) {
                int potential = aPotentials.at(c).at(i);

                if ((f & 1) == 1 && potential >= 0 && potential < min) {
                    min = potential;
                    argmin = c;
                }
            }

            if (argmin < 0) {
                return -1;
            }

            sum += min;
        }

        return sum;
    }

    static int backwardPointwise(const std::vector<std::vector<int>> & aPotentials,
                                 const std::vector<unsigned char> & aPresent)
    {
        int sum = 0;

        for (std::size_t i = 0; i != aPresent.size(); i++) {
            int potential = aPotentials.at(aPresent.at(i)).at(i);

            if (potential < 0) {
                return -1;
            }

            sum += potential;
        }

        return sum;
    }

    unsigned char mFrom = 0;
    int mTo = -1;
};

} // namespace markovjunior
} // namespace ad
