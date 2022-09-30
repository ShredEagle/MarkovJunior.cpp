#pragma once


#include <math/Vector.h>

#include <pugixml.hpp>

#include <iostream>
#include <algorithm>
#include <numeric>
#include <cassert>
#include <map>
#include <vector>

namespace ad {
namespace markovjunior {

inline int getFlatIndex(const math::Position<3, int> & aPos, const math::Size<3, int> & aSize)
{
    return aPos.x() + aPos.y() * aSize.width() + aPos.z() * aSize.width() * aSize.height();
}

class Rule;

class Grid
{
public:
    Grid() = default;
    Grid(const pugi::xml_node & aNode, const math::Size<3, int> & aSize);
    bool matchPatternAtPosition(const Rule & aRule, math::Position<3, int> aPosition) const;
    int makeWave(std::string aValues) const ;
    void clear()
    {
        std::fill(mState.begin(), mState.end(), 0);
    }

    int getFlatGridIndex(const math::Position<3, int> & aPos) const
    {
        return getFlatIndex(aPos, mSize);
    }

    int mTransparent = 0;
    std::map<unsigned char, unsigned char> mValues;
    std::map<unsigned char, int> mWaves;
    std::vector<unsigned char> mCharacters;
    std::vector<unsigned char> mState;
    std::vector<unsigned char> mStateBuffer;
    std::vector<bool> mMask;
    math::Size<3, int> mSize = math::Size<3, int>{1, 1, 1};

    friend std::ostream & operator<<(std::ostream & os, const Grid & aGrid);

    void erase();
};

}
}
