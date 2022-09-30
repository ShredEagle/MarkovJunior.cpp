#include "Map.h"

#include "Interpreter.h"
#include "markovjunior/Commons.h"
#include "markovjunior/Constants.h"
#include "markovjunior/Node.h"
#include "markovjunior/SymmetryUtils.h"

#include <cassert>

namespace ad {
namespace markovjunior {

Scale createMapScale(const pugi::xml_node & aXmlNode)
{
    std::string scaleString = aXmlNode.attribute("scale").as_string("");

    Scale result;

    if (scaleString.compare("") == 0)
    {
        assert(false);
    }

    std::vector<std::string> scales = splitString(scaleString, " ");

    if (scales.size() != 3)
    {
        assert(false);
    }

    for (int i = 0; i < scales.size(); i++)
    {
        std::string scale = scales.at(i);
        if (scale.find("/") == std::string::npos)
        {
            result.mNumerator.at(i) = std::stoi(scale);
            result.mDenominator.at(i) = 1;
        }
    }

    return result;
}

Map::Map(const pugi::xml_node & aXmlNode,
         const SymmetryGroup & aSymmetryGroup,
         Interpreter * aInterpreter,
         Grid * aGrid) :
    SequenceNode(),
    mScale{createMapScale(aXmlNode)},
    mNewGrid{aXmlNode, {
        aGrid->mSize.width() * mScale.mNumerator.x() / mScale.mDenominator.x(),
        aGrid->mSize.height() * mScale.mNumerator.y() / mScale.mDenominator.y(),
        aGrid->mSize.depth() * mScale.mNumerator.z() / mScale.mDenominator.z()
    }}
{
    setupSequenceNode(this, nullptr, aXmlNode, aSymmetryGroup, aInterpreter, &mNewGrid);
    SymmetryGroup symmetry = getSymmetry(aXmlNode.attribute("symmetry").as_string(""), aSymmetryGroup);

    pugi::xpath_node_set xmlRuleNodes = aXmlNode.select_nodes("rule");
    for (pugi::xpath_node aXpathRuleNode : xmlRuleNodes)
    {
        Rule rule{aXpathRuleNode.node(), *aGrid, mNewGrid};
        rule.mOriginal = true;
        std::vector<Rule> symmetricRules = createAllSquareSymmetries(rule, symmetry);
        mRules.insert(mRules.end(), symmetricRules.begin(), symmetricRules.end());
    }
}

bool Map::matches(const Rule &aRule, const math::Position<3, int> &aPos, const std::vector<unsigned char> &aState, const math::Size<3, int> &aSize)
{
    for (int z = 0; z < aRule.mInputSize.depth(); z++)
    {
        for (int y = 0; y < aRule.mInputSize.height(); y++)
        {
            for (int x = 0; x < aRule.mInputSize.width(); x++)
            {
                math::Vec<3, int> shift{x, y, z};
                math::Position<3, int> pos = aPos + shift;

                pos.x() = modulo(pos.x(), aSize.width());
                pos.y() = modulo(pos.y(), aSize.height());
                pos.z() = modulo(pos.z(), aSize.depth());

                int inputWave = aRule.mInputs.at(getFlatIndex(static_cast<math::Position<3, int>>(shift), aRule.mInputSize));
                if ((inputWave & (1 << aState.at(getFlatIndex(pos,  aSize)))) == 0)
                {
                    return false;
                }
            }
        }
    }

    return true;
}

void Map::apply(const Rule &aRule, const math::Position<3, int> &aPos, std::vector<unsigned char> &aState, const math::Size<3, int> &aSize)
{
    for (int z = 0; z < aRule.mOutputSize.depth(); z++)
    {
        for (int y = 0; y < aRule.mOutputSize.height(); y++)
        {
            for (int x = 0; x < aRule.mOutputSize.width(); x++)
            {
                math::Vec<3, int> shift{x, y, z};
                math::Position<3, int> pos = aPos + shift;

                pos.x() = modulo(pos.x(), aSize.width());
                pos.y() = modulo(pos.y(), aSize.height());
                pos.z() = modulo(pos.z(), aSize.depth());

                unsigned char output = aRule.mOutputs.at(getFlatIndex(static_cast<math::Position<3, int>>(shift), aRule.mOutputSize));
                if (output != gWildcardShiftValue)
                {
                    aState.at(getFlatIndex(pos, aSize)) = output;
                }
            }
        }
    }
}

bool Map::run()
{
    if (mCurrentStep >= 0)
    {
        return SequenceNode::run();
    }

    mNewGrid.clear();

    for (const Rule & rule : mRules)
    {
        for (int z = 0; z < mInterpreter->mGrid.mSize.depth(); z++)
        {
            for (int y = 0; y < mInterpreter->mGrid.mSize.height(); y++)
            {
                for (int x = 0; x < mInterpreter->mGrid.mSize.width(); x++)
                {
                    if (matches(rule, {x, y, z}, mInterpreter->mGrid.mState, mInterpreter->mGrid.mSize))
                    {
                        apply(rule, {
                                x * mScale.mNumerator.x() / mScale.mDenominator.x(),
                                y * mScale.mNumerator.y() / mScale.mDenominator.y(),
                                z * mScale.mNumerator.z() / mScale.mDenominator.z()
                                }, mNewGrid.mState, mNewGrid.mSize);
                    }
                }
            }
        }
    }

    mInterpreter->mGrid = std::move(mNewGrid);
    mCurrentStep++;

    return true;
}

} // namespace markovjunior
} // namespace ad
