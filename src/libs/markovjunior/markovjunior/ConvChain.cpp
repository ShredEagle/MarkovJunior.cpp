#include "ConvChain.h"

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

    std::string name = aXmlNode.attribute("sample").as_string();
    auto [samples, size] = loadConvChainSample(
        gResourceLocator.pathFor("assets/resources/" + name + ".png"));

    mSample.reserve(samples.size());
    for (auto s : samples) {
        mSample.emplace_back(s == -1);
    }

    mSubstrate = std::vector<bool>(mGrid->mState.size());

    mWeights = std::vector<double>(1 << (mN * mN), 0.);
    for (int y = 0; y < size.height(); y++) {
        for (int x = 0; x < size.width(); x++) {
            std::vector<bool> patternResult = createPattern<bool>(
                [&, size = size](int dx, int dy) {
                    return mSample.at((x + dx) % size.width()
                                      + (y + dy) % size.height() * size.width());
                },
                mN);
            SamplePattern<bool> samplePattern(patternResult.begin(), patternResult.end());
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

} // namespace markovjunior
} // namespace ad
