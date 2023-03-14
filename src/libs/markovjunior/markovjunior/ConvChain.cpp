#include "ConvChain.h"
#include "markovjunior/Commons.h"       // for SamplePattern, modulo, create...
#include "markovjunior/Grid.h"          // for Grid
#include "markovjunior/ImageHelpers.h"  // for loadConvChainSample
#include "markovjunior/Interpreter.h"   // for Interpreter
#include "markovjunior/Node.h"          // for Node
                                        //
#include <imgui.h>
#include <cassert>                      // for assert
#include <cmath>                        // for pow
#include <cstddef>                      // for size_t
#include <map>                          // for map
#include <pugixml.hpp>                  // for xml_attribute, xml_node, char_t
#include <random>                       // for mt19937, uniform_real_distrib...
#include <cstdio>

namespace ad {
namespace markovjunior {

ConvChain::ConvChain(const pugi::xml_node & aXmlNode,
                     const SymmetryGroup & aSymmetryGroup,
                     Interpreter * aInterpreter,
                     Grid * aGrid) :
    Node(aInterpreter, aGrid),
    mN{aXmlNode.attribute("n").as_int(3)},
    mTemperature{aXmlNode.attribute("temperature").as_double(1.0)},
    mC0{mGrid->mValues.at(aXmlNode.attribute("black").as_string("")[0])},
    mC1{mGrid->mValues.at(aXmlNode.attribute("white").as_string("")[0])},
    mSubstrateColor{
        mGrid->mValues.at(aXmlNode.attribute("on").as_string("")[0])},
    mCounter{0},
    mSteps{aXmlNode.attribute("steps").as_int(-1)}
{
    if (mGrid->mSize.depth() > 1) {
        assert(false);
    }

    const char * name = aXmlNode.attribute("sample").as_string();
    char filePath[256];
    std::snprintf(filePath, 256, "%s/%s%s", "assets/resources/", name, ".png");
    auto [samples, size] = loadConvChainSample(
        aInterpreter->mResourceLocator.pathFor(filePath));

    mSample.reserve(samples.size());
    for (auto s : samples) {
        mSample.push_back((std::byte)(s == -1));
    }

    mSubstrate = std::vector<bool>(mGrid->mState.size());

    mWeights = std::vector<double>((std::size_t)1 << (mN * mN), 0.);
    for (int y = 0; y < size.height(); y++) {
        for (int x = 0; x < size.width(); x++) {
            std::vector<std::byte> patternResult = createPattern(
                [&, size = size](int dx, int dy) {
                    return static_cast<std::byte>(mSample.at((x + dx) % size.width()
                                      + (y + dy) % size.height() * size.width()));
                },
                mN);
            SamplePattern samplePattern(patternResult.begin(), patternResult.end());
            auto symmetries = createAllSquareSymmetries(samplePattern, aSymmetryGroup);

            for (auto sym : symmetries) {
                mWeights.at(sym.index()) += 1.;
            }
        }
    }

    for (auto & weight : mWeights) {
        if (weight <= 0.) {
            weight = 0.1;
        }
    }
}

bool ConvChain::run()
{
    if (mSteps > 0 && mCounter >= mSteps) {
        return false;
    }

    Grid & grid = *mGrid;

    if (mCounter == 0) {
        bool anySubstrate = false;

        for (std::size_t i = 0; i < mSubstrate.size(); i++) {
            if (grid.mState.at(i) == mSubstrateColor) {
                grid.mState.at(i) = mInterpreter->mRandom() % 2 == 0 ? mC0 : mC1;
                mSubstrate.at(i) = true;
                anySubstrate = true;
            }
        }

        mCounter++;
        return anySubstrate;
    }

    for (std::size_t i = 0; i < grid.mState.size(); i++) {
        int r = mInterpreter->mRandom() % grid.mState.size();

        if (!mSubstrate.at(r)) {
            continue;
        }

        int x = r % grid.mSize.width();
        int y = r / grid.mSize.width();
        double q = 1.;

        for (int shiftY = y - mN + 1; shiftY <= y + mN - 1; shiftY++) {
            for (int shiftX = x - mN + 1; shiftX <= x + mN - 1; shiftX++) {
                int ind = 0;
                int difference = 0;

                for (int offsetY = 0; offsetY < mN; offsetY++) {
                    for (int offsetX = 0; offsetX < mN; offsetX++) {
                        int shiftedX = modulo(shiftX + offsetX, grid.mSize.width());
                        int shiftedY = modulo(shiftY + offsetY, grid.mSize.height());

                        bool value =
                            grid.mState.at(grid.getFlatGridIndex({shiftedX, shiftedY, 0}))
                            == mC1;
                        int power = 1 << (offsetY * mN + offsetX);
                        ind += value ? power : 0;
                        if (shiftedX == x && shiftedY == y) {
                            difference = value ? power : -power;
                        }
                    }
                }

                q *= mWeights.at(ind - difference) / mWeights.at(ind);
            }
        }

        if (mTemperature != 1.) {
            q = std::pow(q, 1.0 / mTemperature);
        }

        if (q > mInterpreter->mProbabilityDistribution(mInterpreter->mRandom)) {
            toggle(grid.mState, r);
        }
    }

    mCounter++;

    return true;
};

void ConvChain::debugRender(int id)
{
    ImGui::Text("ConvChain");
}

} // namespace markovjunior
} // namespace ad
