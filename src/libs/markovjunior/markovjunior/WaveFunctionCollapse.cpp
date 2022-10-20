#include "WaveFunctionCollapse.h"

#include "Commons.h"
#include "Interpreter.h"

#include <limits>
#include <random>

namespace ad {
namespace markovjunior {

void Wave::setupWave(int aLength, int aP, int aD, bool)
{
    mData = std::vector<std::vector<bool>>(aLength, std::vector<bool>(aP, true));
    mCompatible = std::vector<std::vector<std::vector<int>>>(
        aLength, std::vector<std::vector<int>>(aP, std::vector<int>(aD, -1)));
}

void Wave::init(const Propagator & aPropagator,
                double aSumOfWeights,
                double aSumOfWeightLogWeights,
                double aStartingEntropy,
                bool aShannon)
{
    mSumsOfOnes = std::vector<int>(mData.size());

    if (aShannon) {
        mSumsOfWeights.clear();
        mSumsOfWeights.reserve(mData.size());
        mSumsOfWeightLogWeights.clear();
        mSumsOfWeightLogWeights.reserve(mData.size());
        mEntropies.clear();
        mEntropies.reserve(mData.size());
    }

    int p = mData.at(0).size();

    for (unsigned int i = 0; i < mData.size(); i++) {
        for (int j = 0; j < p; j++) {
            mData.at(i).at(j) = true;
            for (unsigned int k = 0; k < aPropagator.size(); k++) {
                mCompatible.at(i).at(j).at(k) = aPropagator.at(sOpposite.at(k)).at(j).size();
            }
        }

        mSumsOfOnes.at(i) = p;

        if (aShannon)
        {
            mSumsOfWeights.emplace_back(aSumOfWeights);
            mSumsOfWeightLogWeights.emplace_back(aSumOfWeightLogWeights);
            mEntropies.emplace_back(aStartingEntropy);
        }
    }
}

void Wave::copyFrom(const Wave &aWave, int aD, bool aShannon)
{
    for (unsigned int i = 0; i < mData.size(); i++)
    {
        std::vector<bool> dataItem = mData.at(i);
        std::vector<bool> waveItem = aWave.mData.at(i);

        for (unsigned int j = 0; j < dataItem.size(); j++)
        {
            dataItem.at(j) = waveItem.at(j);

            for (int k = 0; k < aD; k++)
            {
                mCompatible.at(i).at(j).at(k) = aWave.mCompatible.at(i).at(j).at(k);
            }
        }

        mSumsOfOnes.at(i) = aWave.mSumsOfOnes.at(i);

        if (aShannon)
        {
            mSumsOfWeights.at(i) = aWave.mSumsOfWeights.at(i);
            mSumsOfWeightLogWeights.at(i) = aWave.mSumsOfWeightLogWeights.at(i);
            mEntropies.at(i) = aWave.mEntropies.at(i);
        }
    }
}

void WFCNode::setupWFCNode(const pugi::xml_node & aXmlNode,
                           const SymmetryGroup & aParentSymmetry,
                           Interpreter * aInterpreter,
                           Grid * aGrid)
{
    mWave.setupWave(aGrid->mState.size(), mP, mPropagator.size(), mShannon);
    mStartWave = mWave;

    if (mShannon) {
        mWeightLogWeights.reserve(mP);

        for (auto weight : mWeights) {
            mWeightLogWeights.push_back(weight * std::log(weight));
            mSumOfWeights += weight;
            mSumOfWeightLogWeights += mWeightLogWeights.back();
        }

        mStartingEntropy =
            std::log(mSumOfWeights) - mSumOfWeightLogWeights / mSumOfWeights;
    }

    mDistribution.reserve(mP);

    setupSequenceNode(this, aXmlNode, aParentSymmetry, aInterpreter, &mNewGrid);
}

void WFCNode::ban(int aIndex, int aT)
{
    mWave.mData.at(aIndex).at(aT) = false;

    std::vector<int> & compatibility = mWave.mCompatible.at(aIndex).at(aT);

    for (unsigned int i = 0; i < mPropagator.size(); i++) {
        compatibility.at(i) = 0;
    }

    mStack.emplace(aIndex, aT);

    mWave.mSumsOfOnes.at(aIndex) -= 1;
    if (mShannon) {
        double sum = mWave.mSumsOfWeights.at(aIndex);
        mWave.mEntropies.at(aIndex) +=
            mWave.mSumsOfWeightLogWeights.at(aIndex) / sum - std::log(sum);

        mWave.mSumsOfWeights.at(aIndex) -= mWeights.at(aT);
        mWave.mSumsOfWeightLogWeights.at(aIndex) -= mWeightLogWeights.at(aT);

        sum = mWave.mSumsOfWeights.at(aIndex);
        mWave.mEntropies.at(aIndex) -=
            mWave.mSumsOfWeightLogWeights.at(aIndex) / sum - std::log(sum);
    }
}

bool WFCNode::propagate()
{
    while (mStack.size() > 0) {
        auto [i1, p1] = mStack.top();
        mStack.pop();

        math::Position<3, int> pos1 = {
            i1 % mGrid->mSize.width(),
            i1 % (mGrid->mSize.width() * mGrid->mSize.height()) / mGrid->mSize.width(),
            i1 / (mGrid->mSize.width() * mGrid->mSize.height())};

        for (unsigned int i = 0; i < mPropagator.size(); i++) {
            math::Vec<3, int> shift = {sDx.at(i), sDy.at(i), sDz.at(i)};

            math::Position<3, int> pos2 = pos1 + shift;

            if (!mPeriodic
                && (pos2.x() < 0 || pos2.y() < 0 || pos2.z() < 0
                    || pos2.x() + mN > mGrid->mSize.width()
                    || pos2.y() + mN > mGrid->mSize.height()
                    || pos2.z() + 1 > mGrid->mSize.depth())) {
                continue;
            }

            pos2.x() = modulo(pos2.x(), mGrid->mSize.width());
            pos2.y() = modulo(pos2.y(), mGrid->mSize.height());
            pos2.z() = modulo(pos2.z(), mGrid->mSize.depth());

            int flatIndex2 = getFlatIndex(pos2, mGrid->mSize);

            const std::vector<int> & propagator = mPropagator.at(i).at(p1);
            std::vector<std::vector<int>> & compatibility =
                mWave.mCompatible.at(flatIndex2);

            for (auto prop : propagator) {
                int t2 = prop;
                std::vector<int> & c = compatibility.at(t2);

                c.at(i)--;
                if (c.at(i) == 0) {
                    ban(flatIndex2, t2);
                }
            }
        }
    }

    return mWave.mSumsOfOnes.at(0) > 0;
}

void WFCNode::observe(int aNode)
{
    std::vector<bool> waveData = mWave.mData.at(aNode);
    for (int i = 0; i < mP; i++) {
        mDistribution.at(i) = waveData.at(i) ? mWeights.at(i) : 0.0;
    }

    int r = getRandomIndex(mDistribution,
                           mInterpreter->mProbabilityDistribution(mLocalRandom));
    for (int j = 0; j < mP; j++) {
        if (waveData.at(j) != (j == r)) {
            ban(aNode, j);
        }
    }
}

int WFCNode::nextUnobservedNode()
{
    double min = std::numeric_limits<double>::max();
    int argmin = -1;

    for (int z = 0; z < mGrid->mSize.depth(); z++) {
        for (int y = 0; y < mGrid->mSize.height(); y++) {
            for (int x = 0; x < mGrid->mSize.width(); x++) {
                if (!mPeriodic
                    && (x + mN > mGrid->mSize.width() || y + mN > mGrid->mSize.height()
                        || z + 1 > mGrid->mSize.depth())) {
                    continue;
                }

                int flatIndex = getFlatIndex({x, y, z}, mGrid->mSize);
                int remainingValues = mWave.mSumsOfOnes.at(flatIndex);
                double entropy =
                    mShannon ? mWave.mEntropies.at(flatIndex) : remainingValues;

                if (remainingValues > 1 && entropy <= min) {
                    double noise =
                        0.000001 * mInterpreter->mProbabilityDistribution(mLocalRandom);
                    if (entropy + noise < min) {
                        min = entropy + noise;
                        argmin = flatIndex;
                    }
                }
            }
        }
    }

    return argmin;
}

std::optional<int> WFCNode::getGoodSeed()
{
    for (int i = 0; i < mTries; i++) {
        int observationSoFar = 0;
        int seed = mInterpreter->mRandom();
        mLocalRandom = std::mt19937(seed);

        mStack = std::stack<std::pair<int, int>>();

        mWave.copyFrom(mStartWave, mPropagator.size(), mShannon);

        while (true) {
            int node = nextUnobservedNode();

            if (node >= 0) {
                observe(node);
                observationSoFar++;
                bool success = propagate();

                if (!success) {
                    break;
                }
            } else {
                return seed;
            }
        }
    }
    return std::nullopt;
}

bool WFCNode::run()
{
    if (mN >= 0) {
        return SequenceNode::run();
    }

    if (mFirstGo) {
        mWave.init(mPropagator, mSumOfWeights, mSumOfWeightLogWeights, mStartingEntropy,
                   mShannon);

        for (unsigned int i = 0; i < mWave.mData.size(); i++) {
            unsigned char value = mGrid->mState.at(i);

            if (mMap.contains(value)) {
                std::vector<bool> boolStartWave = mMap.at(value);
                for (int j = 0; j < mP; j++) {
                    if (!boolStartWave.at(j)) {
                        ban(i, j);
                    }
                }
            }
        }

        bool firstSuccess = propagate();

        if (!firstSuccess) {
            return false;
        }

        mStartWave.copyFrom(mWave, mPropagator.size(), mShannon);

        std::optional<int> goodseed = getGoodSeed();
        if (!goodseed) {
            return false;
        }
        mLocalRandom = std::mt19937(goodseed.value());
        mFirstGo = false;

        mNewGrid.clear();
        mInterpreter->mGrid = std::move(mNewGrid);
        return true;
    } else {
        int node = nextUnobservedNode();
        if (node >= 0) {
            observe(node);
            propagate();
        } else {
            mN++;
        }

        if (mN >= 0) {
            updateState();
        }

        return true;
    }
}

} // namespace markovjunior
} // namespace ad
