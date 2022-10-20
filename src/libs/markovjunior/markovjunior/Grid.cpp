#include "Grid.h"
#include "Rule.h"
#include <iostream>

namespace ad {
namespace markovjunior {
Grid::Grid(const pugi::xml_node & aNode, const math::Size<3, int> & aSize) :
        mState(aSize.width() * aSize.depth() * aSize.height(), 0),
        mStateBuffer(aSize.width() * aSize.depth() * aSize.height(), 0),
        mMask(aSize.width() * aSize.depth() * aSize.height(), false),
        mSize{aSize}
{
    std::string valuesString = aNode.attribute("values").as_string("");
    assert(!valuesString.empty());
    for (unsigned int i = 0; i < valuesString.size(); i++)
    {
        mCharacters.push_back(valuesString.at(i));
        mValues.insert({valuesString.at(i), i});
        mWaves.insert({valuesString.at(i), 1 << i});
    }

    std::string transparentString = aNode.attribute("transparent").as_string("");


    if (!transparentString.empty())
    {
        mTransparent = makeWave(transparentString);
    }

    pugi::xpath_node_set allUnions = aNode.select_nodes("//union");

    mWaves.insert({"*"[0], (1 << mCharacters.size()) - 1});
    for (auto xpathNode : allUnions)
    {
        unsigned char symbol = xpathNode.node().attribute("symbol").as_string()[0];
        mWaves.insert({symbol, makeWave(xpathNode.node().attribute("values").as_string())});
    }

}

// IMPORTANT TODO !!!!
// This matching algorithm seems garbo
bool Grid::matchPatternAtPosition(const Rule & aRule, math::Position<3, int> aPosition) const
{
    math::Vec<3, int> offset{math::Vec<3, int>::Zero()};
    for (auto input: aRule.mInputs)
    {
        if (
            (input &
                (1 << mState.at(
                    getFlatGridIndex(aPosition + offset)
                ))) == 0
            )
        {
            return false;
        }

        offset.x()++;
        if (offset.x() == aRule.mInputSize.width())
        {
            offset.x() = 0;
            offset.y()++;

            if (offset.y() == aRule.mInputSize.height())
            {
                offset.y() = 0;
                offset.z()++;
            }
        }
    }

    return true;
}

int Grid::makeWave(std::string aValues) const
{
    std::vector<int> mappedValues;
    std::for_each(aValues.begin(), aValues.end(), [&](unsigned char value) {mappedValues.emplace_back(1 << mValues.at(value));});
    return std::accumulate(mappedValues.begin(), mappedValues.end(), 0);
}

std::ostream &operator<<(std::ostream & os, const Grid & aGrid)
{
    os << "GRID STATE" << std::endl;

    for (int k = 0; k != aGrid.mSize.depth(); k++)
    {
        for (int j = 0; j != aGrid.mSize.height(); j++)
        {
            for (int i = 0; i != aGrid.mSize.width(); i++)
            {
                int index = aGrid.getFlatGridIndex({i, j, k});
                os << aGrid.mCharacters.at(aGrid.mState.at(index)) << " ";
            }
            os << std::endl;
        }
    }

    os << std::endl;

    return os;
}

void Grid::erase()
{
    std::cout << "\x1b[1A" << "\x1b[2K";
    for (int k = 0; k != mSize.depth(); k++)
    {
        for (int j = 0; j != mSize.height(); j++)
        {
            std::cout << "\x1b[1A" << "\x1b[2K";
        }
    }
    std::cout << "\x1b[1A" << "\x1b[2K";
}
}
}
