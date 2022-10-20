#pragma once

#include "markovjunior/Node.h"

#include <optional>
#include <pugixml.hpp>

#include <random>
#include <stack>
#include <utility>
#include <vector>

namespace ad {
namespace markovjunior {

typedef std::vector<std::vector<std::vector<int>>> Propagator;

class Wave
{
  public:
    static constexpr std::array<int, 6> sOpposite = {2, 3, 0, 1, 5, 4};

    Wave() = default;
    void setupWave(int aLength, int aP, int aD, bool aShannon);
    void init(const Propagator & aPropagator,
              double aSumOfWeights,
              double aSumOfWeightLogWeights,
              double aStartingEntropy,
              bool aShannon);
    void copyFrom(const Wave & aWave,
            int aD,
            bool aShannon);

    std::vector<std::vector<bool>> mData;
    std::vector<std::vector<std::vector<int>>> mCompatible;
    std::vector<int> mSumsOfOnes;
    std::vector<double> mSumsOfWeights;
    std::vector<double> mSumsOfWeightLogWeights;
    std::vector<double> mEntropies;
};

class WFCNode : public SequenceNode
{
    public:
        Wave mWave;
        Propagator mPropagator;
        int mP;
        int mN = 1;
        std::stack<std::pair<int, int>> mStack;
        int mStackSize;

        std::vector<double> mWeights;
        std::vector<double> mWeightLogWeights;
        double mSumOfWeights = 0;
        double mSumOfWeightLogWeights = 0;
        double mStartingEntropy = 0;

        Grid mNewGrid;
        Wave mStartWave;

        std::map<unsigned char, std::vector<bool>> mMap;
        bool mPeriodic;
        bool mShannon;

        std::vector<double> mDistribution;
        int mTries;

        std::string mName;

        bool mFirstGo = true;
        std::mt19937 mLocalRandom;

        virtual void updateState() = 0;

        static constexpr std::array<int, 6> sDx = {1, 0, -1, 0, 0, 0};
        static constexpr std::array<int, 6> sDy = {0, 1, 0, -1, 0, 0};
        static constexpr std::array<int, 6> sDz = {0, 0, 0, 0, 1, -1};

        WFCNode(const pugi::xml_node & aXmlNode,
                const SymmetryGroup &,
                Interpreter *,
                Grid *) :
            SequenceNode(),
            mShannon{aXmlNode.attribute("shannon").as_bool(false)},
            mTries{aXmlNode.attribute("tries").as_int(1000)}
        {
        }

        void setupWFCNode(const pugi::xml_node & aXmlNode,
                const SymmetryGroup & aParentSymmetry,
                Interpreter * aInterpreter,
                Grid * aGrid);
        void ban(int aIndex, int aT);
        bool propagate();
        void observe(int aNode);
        int nextUnobservedNode();
        std::optional<int> getGoodSeed();
        bool run() override;

        int getRandomIndex(const std::vector<double> & aWeights, double r)
        {
            double sum = std::accumulate(aWeights.begin(), aWeights.end(), 0.);
            double threshold = r * sum;
            double partialSum = 0.;

            for (unsigned int i = 0; i < aWeights.size(); i++)
            {
                partialSum += aWeights.at(i);
                if (partialSum >= threshold)
                {
                    return i;
                }
            }

            return 0;
        }
        
        void reset() override
        {
            SequenceNode::reset();
            mCurrentStep = -1;
            mFirstGo = true;
        }
};
} // namespace markovjunior
} // namespace ad
