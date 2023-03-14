#include "Observation.h"

#include "markovjunior/Constants.h"
#include "markovjunior/Rule.h"

namespace ad {
namespace markovjunior {

Observation::Observation(const unsigned char aFrom, const std::string & aTo, const Grid & aGrid) :
    mFrom{aGrid.mValues.at(aFrom)}, mTo{aGrid.makeWave(aTo)}
{
}

bool Observation::computeFutureSetPresent(std::vector<int> & aFuture,
                                          std::vector<unsigned char> & aState,
                                          const std::vector<Observation> & aObservations)
{
    std::vector<bool> mask(aObservations.size(), false);

    for (unsigned int i = 0; i < aObservations.size(); i++) {
        mask.at(i) = aObservations.at(i).mTo == -1;
    }

    for (unsigned int i = 0; i < aState.size(); i++) {
        unsigned char value = aState.at(i);
        const Observation obs = aObservations.at(value);
        mask.at(value) = true;

        if (obs.mTo != -1) {
            aFuture.at(i) = obs.mTo;
            aState.at(i) = obs.mFrom;
        } else {
            aFuture.at(i) = 1 << value;
        }
    }

    for (auto maskValue : mask)
    {
        if (!maskValue)
        {
            return false;
        }
    }

    return true;
}

void Observation::computePotentials(std::vector<std::vector<int>> & aPotentials,
                                    const math::Size<3, int> & aSize,
                                    const std::vector<Rule> & aRules,
                                    bool aBackwards)
{
    std::queue<std::tuple<unsigned char, math::Position<3, int>>> posQueue;

    for (unsigned char c = 0; c < aPotentials.size(); c++) {
        std::vector<int> potentialField = aPotentials.at(c);

        for (unsigned int i = 0; i < potentialField.size(); i++) {
            if (potentialField.at(i) == 0) {
                posQueue.emplace(
                    c, math::Position<3, int>{
                           static_cast<int>(i % aSize.width()),
                           static_cast<int>((i % (aSize.width() * aSize.height())) / aSize.width()),
                           static_cast<int>(i / (aSize.width() * aSize.height()))});
            }
        }
    }

    std::vector<std::vector<bool>> matchMask = std::vector<std::vector<bool>>(
        aRules.size(), std::vector<bool>(aPotentials.at(0).size(), false));

    while (!posQueue.empty()) {
        auto [value, pos] = posQueue.front();
        posQueue.pop();
        int flatIndex = getFlatIndex(pos, aSize);
        int potentialValue = aPotentials.at(value).at(flatIndex);

        for (unsigned int ruleIndex = 0; ruleIndex < aRules.size(); ruleIndex++) {
            std::vector<bool> & ruleMask = matchMask.at(ruleIndex);

            const Rule & rule = aRules.at(ruleIndex);
            const auto & shifts =
                aBackwards ? rule.mOutputShifts.at(value) : rule.mInputShifts.at(value);

            for (auto shiftPos : shifts) {
                math::Position<3, int> shiftedPos = pos - shiftPos;

                if (shiftedPos.x() < 0 || shiftedPos.y() < 0 || shiftedPos.z() < 0
                    || shiftedPos.x() + rule.mInputSize.width() > aSize.width()
                    || shiftedPos.y() + rule.mInputSize.height() > aSize.height()
                    || shiftedPos.z() + rule.mInputSize.depth() > aSize.depth()) {
                    continue;
                }

                int shiftedFlatIndex = getFlatIndex(shiftedPos, aSize);

                if (!ruleMask.at(shiftedFlatIndex)
                    && forwardMatches(rule, shiftedPos, aPotentials, potentialValue,
                                      aSize, aBackwards)) {
                    ruleMask.at(shiftedFlatIndex) = true;
                    applyForward(rule, shiftedPos, aPotentials, potentialValue, aSize,
                                 posQueue, aBackwards);
                }
            }
        }
    }
}

bool Observation::forwardMatches(const Rule & aRule,
                                 const math::Position<3, int> & aPos,
                                 const std::vector<std::vector<int>> & aPotentials,
                                 int aThreshold,
                                 const math::Size<3, int> & aSize,
                                 bool aBackwards)
{
    std::vector<unsigned char> values = aBackwards ? aRule.mOutputs : aRule.mByteInput;
    for (int z = 0; z < aRule.mInputSize.depth(); z++) {
        for (int y = 0; y < aRule.mInputSize.height(); y++) {
            for (int x = 0; x < aRule.mInputSize.width(); x++) {

                int flatIndex = getFlatIndex({x, y, z}, aRule.mInputSize);
                unsigned char value = values.at(flatIndex);

                if (value != gWildcardShiftValue) {
                    int current = aPotentials.at(value).at(
                        getFlatIndex(aPos + math::Vec<3, int>{x, y, z}, aSize));
                    if (current > aThreshold || current == -1) {
                        return false;
                    }
                }

            }
        }
    }

    return true;
}

void Observation::applyForward(
    const Rule & aRule,
    const math::Position<3, int> & aPos,
    std::vector<std::vector<int>> & aPotentials,
    int aThreshold,
    const math::Size<3, int> & aSize,
    std::queue<std::tuple<unsigned char, math::Position<3, int>>> & aQueue,
    bool aBackwards)
{
    std::vector<unsigned char> values = aBackwards ? aRule.mByteInput : aRule.mOutputs;

    for (int z = 0; z < aRule.mInputSize.depth(); z++) {
        for (int y = 0; y < aRule.mInputSize.height(); y++) {
            for (int x = 0; x < aRule.mInputSize.width(); x++) {
                math::Position<3, int> shiftedPos = aPos + math::Vec<3, int>{x, y, z};
                int gridFlatIndex = getFlatIndex(shiftedPos, aSize);
                int ruleFlatIndex = getFlatIndex({x, y, z}, aRule.mInputSize);

                unsigned char value = values.at(ruleFlatIndex);

                if (value != gWildcardShiftValue
                    && aPotentials.at(value).at(gridFlatIndex) == -1) {
                    aPotentials.at(value).at(gridFlatIndex) = aThreshold + 1;
                    aQueue.emplace(value, shiftedPos);
                }
            }
        }
    }
}

} // namespace markovjunior
} // namespace ad
