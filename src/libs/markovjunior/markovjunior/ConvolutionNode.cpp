#include "ConvolutionNode.h"

#include "Commons.h"
#include "Grid.h"           // for Grid
#include "Interpreter.h"    // for Interpreter
#include "Node.h"           // for Node
#include "SymmetryUtils.h"  // for SymmetryGroup

#include <math/MatrixBase.h>             // for operator+, MatrixBase
#include <math/Vector.h>                 // for Position, Size, Vec

#include <pugixml.hpp>                   // for xml_node, xml_attribute, char_t
#include <iostream>
#include <string>
#include <random>                        // for mt19937, uniform_real_distri...

namespace ad {
namespace markovjunior {

ConvolutionRule::ConvolutionRule(const pugi::xml_node & aXmlNode, const Grid & aGrid) :
    mInput{aGrid.mValues.at(aXmlNode.attribute("in").as_string("")[0])},
    mOutput{aGrid.mValues.at(aXmlNode.attribute("out").as_string("")[0])},
    mProbability{aXmlNode.attribute("p").as_double(1.0)}
{
    std::string valueString = aXmlNode.attribute("values").as_string("");
    std::string sumsString = aXmlNode.attribute("sum").as_string("");

    if (!valueString.compare("") && sumsString.compare(""))
    {
        std::cout << "missing sum attribute for convolution rule" << std::endl;
    }
    if (valueString.compare("") && !sumsString.compare(""))
    {
        std::cout << "missing values attribute for convolution rule" << std::endl;
    }

    if (valueString.compare("") != 0)
    {
        for (unsigned char character : valueString)
        {
            mValues.push_back(aGrid.mValues.at(character));
        }

        mSums = std::vector<bool>(28, false);

        for (int i : splitIntervals(sumsString))
        {
            mSums.at(i) = true;
        }
    }
}

ConvolutionNode::ConvolutionNode(const pugi::xml_node & aXmlNode, const SymmetryGroup & aParentSymmetryGroup, Interpreter * aInterpreter, Grid * aGrid) :
    Node(aInterpreter, aGrid),
    mSteps{aXmlNode.attribute("steps").as_int(-1)},
    mPeriodic{aXmlNode.attribute("periodic").as_bool(false)}
{
    pugi::xpath_node_set rules = aXmlNode.select_nodes("rule");

    for (pugi::xpath_node ruleNode : rules)
    {
        mRules.emplace_back(ruleNode.node(), mInterpreter->mGrid);
    }

    std::string neighborhood = aXmlNode.attribute("neighborhood").as_string("");

    if (mGrid->mSize.depth() == 1)
    {
        mKernel = std::vector<int>{gTwoDKernels.at(neighborhood).begin(), gTwoDKernels.at(neighborhood).end()};
    }
    else
    {
        mKernel = std::vector<int>{gThreeDKernels.at(neighborhood).begin(), gThreeDKernels.at(neighborhood).end()};
    }
}

bool ConvolutionNode::run()
{
    if (mSteps > 0 && mCounter >= mSteps)
    {
        return false;
    }

    mSumField = std::vector<std::vector<int>>(mGrid->mState.size(), std::vector<int>(mGrid->mCharacters.size(), 0));

    Grid & grid = *mGrid;
    const auto & gridSize = grid.mSize;

    for (int z = 0; z < gridSize.depth(); z++)
    {
        for (int y = 0; y < gridSize.height(); y++)
        {
            for (int x = 0; x < gridSize.width(); x++)
            {
                std::vector<int> & sums = mSumField.at(grid.getFlatGridIndex({x, y, z}));

                for (int offsetZ = -1; offsetZ <= 1; offsetZ++)
                {
                    for (int offsetY = -1; offsetY <= 1; offsetY++)
                    {
                        for (int offsetX = -1; offsetX <= 1; offsetX++)
                        {
                            math::Position<3, int> offsetPos = math::Position<3, int>{x, y, z} + math::Vec<3, int>{offsetX, offsetY, offsetZ};

                            if (mPeriodic)
                            {
                                offsetPos.x() = modulo(offsetPos.x(), gridSize.width());
                                offsetPos.y() = modulo(offsetPos.y(), gridSize.height());
                                offsetPos.z() = modulo(offsetPos.z(), gridSize.depth());
                            }
                            else if (offsetPos.x() < 0 ||
                                    offsetPos.y() < 0 ||
                                    offsetPos.z() < 0 ||
                                    offsetPos.x() >= gridSize.width() ||
                                    offsetPos.y() >= gridSize.height() ||
                                    offsetPos.z() >= gridSize.depth()
                                    )
                            {
                                continue;
                            }

                            sums.at(grid.mState.at(grid.getFlatGridIndex(offsetPos))) += mKernel.at(offsetX + 1 + (offsetY + 1) * 3 + (offsetZ + 1) * 9);
                        }
                    }
                }
            }
        }
    }

    bool change = false;

    for (int i = 0; i < mSumField.size(); i++)
    {
        std::vector<int> sums = mSumField.at(i);
        unsigned char input = grid.mState.at(i);

        for (ConvolutionRule rule : mRules)
        {
            mInterpreter->mRandom();
            if (input == rule.mInput && rule.mOutput != grid.mState.at(i) && mInterpreter->mProbabilityDistribution(mInterpreter->mRandom) < rule.mProbability)
            {
                bool success = true;

                if (rule.mSums.size() > 0)
                {
                    int sum = 0;

                    for (auto value : rule.mValues)
                    {
                        sum += sums.at(value);
                    }

                    success = rule.mSums.at(sum);
                }

                if (success)
                {
                    grid.mState.at(i) = rule.mOutput;
                    change = true;
                    break;
                }
            }
        }
    }

    mCounter++;
    return change;
}

}
}
