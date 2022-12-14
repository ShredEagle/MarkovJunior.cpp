#pragma once

#include "Grid.h"

#include <math/Vector.h>

#include <optional>
#include <pugixml.hpp>
#include <algorithm>
#include <vector>
#include <string>

namespace ad {
namespace markovjunior {

class Rule;

class Field
{
public:
    Field() = default;

    Field(const pugi::xml_node & aXmlNode, const Grid & aGrid) :
        mRecompute{aXmlNode.attribute("recompute").as_bool(false)},
        mEssential{aXmlNode.attribute("essential").as_bool(false)},
        mSubstrate{aGrid.makeWave(aXmlNode.attribute("on").as_string())}
    {

        std::string zeroSymbols = aXmlNode.attribute("from").as_string("");
        if (!zeroSymbols.empty())
        {
            mInversed = true;
        }
        else
        {
            zeroSymbols = aXmlNode.attribute("to").as_string();
        }

        mZero = aGrid.makeWave(zeroSymbols);
    }

    bool compute(std::vector<int> & aPotential, const Grid & aGrid);

    std::vector<math::Position<3, int>> getNeighbors(const math::Position<3, int> & aPos, const math::Size<3, int> & aSize)
    {
        std::vector<math::Position<3, int>> neighbors;

        if (aPos.x() > 0)
        {
            neighbors.emplace_back(aPos.x() - 1, aPos.y(), aPos.z());
        }
        if (aPos.x() < aSize.width() - 1)
        {
            neighbors.emplace_back(aPos.x() + 1, aPos.y(), aPos.z());
        }
        if (aPos.y() > 0)
        {
            neighbors.emplace_back(aPos.x(), aPos.y() - 1, aPos.z());
        }
        if (aPos.y() < aSize.height() - 1)
        {
            neighbors.emplace_back(aPos.x(), aPos.y() + 1, aPos.z());
        }
        if (aPos.z() > 0)
        {
            neighbors.emplace_back(aPos.x(), aPos.y(), aPos.z() - 1);
        }
        if (aPos.z() < aSize.depth() - 1)
        {
            neighbors.emplace_back(aPos.x(), aPos.y(), aPos.z() + 1);
        }

        return neighbors;
    }

    static std::optional<int> deltaPointwise(
            const std::vector<unsigned char> & aState,
            const Rule & aRule,
            const math::Position<3, int> & aPos,
            const std::vector<Field> & aFields,
            const std::vector<std::vector<int>> & aPotentials,
            const math::Size<3, int> & aGridSize);

    bool mRecompute = false;
    bool mInversed = false;
    bool mEssential = false;
    int mZero = 0;
    int mSubstrate = -1;
};
}
}
