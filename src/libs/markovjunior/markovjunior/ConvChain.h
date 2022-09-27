#pragma once

#include "Node.h"
#include "Interpreter.h"
#include "markovjunior/Commons.h"
#include "markovjunior/ImageHelpers.h"

#include <cmath>
#include <functional>
#include <math/Vector.h>

#include <pugixml.hpp>
#include <cassert>

namespace ad {
namespace markovjunior {

template<class T>
std::vector<T> createPattern(std::function<T(int, int)> aFunction, int aN)
{
    std::vector<T> result(aN * aN);
    for (int y = 0; y < aN; y++)
    {
        for (int x = 0; x < aN; x++)
        {
            result.at(x + y * aN) = aFunction(x, y);
        }
    }
    return result;
}

template<class T_sampleType>
struct SamplePattern : public std::vector<T_sampleType>
{
    using std::vector<T_sampleType>::vector;

    SamplePattern<T_sampleType> rotate()
    {
        std::vector<T_sampleType> result = createPattern<T_sampleType>([&](int x, int y) { return this->at(sqrt(this->size()) - 1 - y + x * sqrt(this->size()));}, sqrt(this->size()));
        return SamplePattern<T_sampleType>(result.begin(), result.end());
    }
    SamplePattern<T_sampleType> reflect()
    {
        std::vector<T_sampleType> result = createPattern<T_sampleType>([&](int x, int y) { return this->at(sqrt(this->size()) - 1 - x + y * sqrt(this->size()));}, sqrt(this->size()));
        return SamplePattern<T_sampleType>(result.begin(), result.end());
    }
    int index()
    {
        int result = 0;
        int power = 1;

        for (int i = 0; i < this->size(); i++, power *= 2)
        {
            if (this->at(i))
            {
                result += power;
            }
        }

        return result;
    }
};

class ConvChain : public Node
{
public:
    int mN;
    double mTemperature;
    std::vector<double> mWeights;
    unsigned char mC0;
    unsigned char mC1;
    std::vector<bool> mSubstrate;
    unsigned char mSubstrateColor;
    int mCounter;
    int mSteps;
    SamplePattern<bool> mSample;
    math::Size<3, int> mSize;

    ConvChain(const pugi::xml_node & aXmlNode, const SymmetryGroup & aSymmetryGroup, Interpreter * aInterpreter) :
        Node(aInterpreter),
        mN{aXmlNode.attribute("n").as_int(3)},
        mTemperature{aXmlNode.attribute("temperature").as_double(1.0)},
        mC0{mInterpreter->mGrid.mValues.at(aXmlNode.attribute("black").as_string("")[0])},
        mC1{mInterpreter->mGrid.mValues.at(aXmlNode.attribute("white").as_string("")[0])},
        mSubstrateColor{mInterpreter->mGrid.mValues.at(aXmlNode.attribute("on").as_string("")[0])},
        mSteps{aXmlNode.attribute("steps").as_int(-1)}
    {
        if (mInterpreter->mGrid.mSize.depth() > 1)
        {
            assert(false);
        }

        std::string name = aXmlNode.attribute("sample").as_string();
        auto [samples, size] = loadConvChainSample(gResourceLocator.pathFor("assets/resources/" + name + ".png"));

        mSample.reserve(samples.size());
        for (auto s : samples)
        {
            mSample.emplace_back(s == -1);
        }

        mSubstrate = std::vector<bool>(mInterpreter->mGrid.mState.size());

        mWeights = std::vector<double>(1 << (mN * mN), 0.);
        for (int y = 0; y < size.height(); y++)
        {
            for (int x = 0; x < size.width(); x++)
            {
                std::vector<bool> patternResult = createPattern<bool>([&, size = size](int dx, int dy) {
                        return mSample.at((x + dx) % size.width() + (y + dy) % size.height() * size.width());
                    }, mN);
                SamplePattern<bool> samplePattern(patternResult.begin(), patternResult.end());
                auto symmetries = createAllSquareSymmetries(samplePattern, aSymmetryGroup);

                for (auto sym : symmetries)
                {
                    mWeights.at(sym.index()) += 1.;
                }
            }
        }

        for (auto & weight : mWeights)
        {
            if(weight <= 0.)
            {
                weight = 0.1;
            }
        }
    }

    void toggle(std::vector<unsigned char> & aState, int i)
    {
        aState.at(i) = aState.at(i) == mC0 ? mC1 : mC0;
    }

    void reset() override
    {
        mCounter = 0;

        std::fill(mSubstrate.begin(), mSubstrate.end(), false);
    };

    bool run() override
    {
        if (mSteps > 0; mCounter >= mSteps)
        {
            return false;
        }

        Grid & grid = mInterpreter->mGrid;

        if (mCounter == 0)
        {
            bool anySubstrate = false;

            for (int i = 0; i < mSubstrate.size(); i++)
            {
                if (grid.mState.at(i) == mSubstrateColor)
                {
                    grid.mState.at(i) = mInterpreter->mRandom() % 2 == 0 ? mC0 : mC1;
                    mSubstrate.at(i) = true;
                    anySubstrate = true;
                }
            }

            mCounter++;
            return anySubstrate;
        }

        for (int i = 0; i < grid.mState.size(); i++)
        {
            int r = mInterpreter->mRandom() % grid.mState.size();

            if (!mSubstrate.at(r))
            {
                continue;
            }

            int x = r % grid.mSize.width();
            int y = r / grid.mSize.width();
            double q = 1.;

            for (int shiftY = y - mN + 1; shiftY <= y + mN - 1; shiftY++)
            {
                for (int shiftX = x - mN + 1; shiftX <= x + mN - 1; shiftX++)
                {
                    int ind = 0;
                    int difference = 0;

                    for (int offsetY = 0; offsetY < mN; offsetY++)
                    {
                        for (int offsetX = 0; offsetX < mN; offsetX++)
                        {
                            int shiftedX = modulo(shiftX + offsetX, grid.mSize.width());
                            int shiftedY = modulo(shiftY + offsetY, grid.mSize.height());

                            bool value = grid.mState.at(grid.getFlatGridIndex({shiftedX, shiftedY, 0})) == mC1;
                            int power = 1 << (offsetY * mN + offsetX);
                            ind += value ? power : 0;
                            if (shiftedX == x && shiftedY == y)
                            {
                                difference = value ? power : -power;
                            }
                        }
                    }

                    q *= mWeights.at(ind - difference) / mWeights.at(ind);
                }
            }

            if (mTemperature != 1.)
            {
                q = std::pow(q, 1.0 / mTemperature);
            }

            if (q > mInterpreter->mProbabilityDistribution(mInterpreter->mRandom))
            {
                toggle(grid.mState, r);
            }
        }

        mCounter++;

        return true;
    };
};

}
}
