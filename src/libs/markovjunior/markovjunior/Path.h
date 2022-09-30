#pragma once

#include "Node.h"

#include "Interpreter.h"

#include <math/Vector.h>
#include <pugixml.hpp>
#include <queue>
#include <string>

namespace ad {
namespace markovjunior {

class Path : public Node
{
public:
    Path(const pugi::xml_node & aXmlNode, const SymmetryGroup & aSymmetryGroup, Interpreter * aInterpreter, Grid * aGrid) :
        Node(aInterpreter, aGrid),
        mInertia{aXmlNode.attribute("inertia").as_bool()},
        mLongest{aXmlNode.attribute("longest").as_bool()},
        mEdges{aXmlNode.attribute("edges").as_bool()},
        mVertices{aXmlNode.attribute("vertices").as_bool()}
    {
        std::string startSymbols = aXmlNode.attribute("from").as_string("");
        const Grid & grid = *mGrid;
        mStart = grid.makeWave(startSymbols);
        mValue = grid.mValues.at(aXmlNode.attribute("color").as_string(startSymbols.substr(0, 1).c_str())[0]);
        mFinish = grid.makeWave(aXmlNode.attribute("to").as_string(""));
        mSubstrate = grid.makeWave(aXmlNode.attribute("on").as_string(""));
    }

    void reset() override
    {}

    bool run() override;
    math::Vec<3, int> getDirection(
            const math::Position<3, int> & aPos,
            const math::Vec<3, int> & aDirection,
            const std::vector<int> & aGenerations,
            std::mt19937 & aRandom
            );
    std::vector<math::Vec<3, int>> getAllPossibleDirections(
            const math::Position<3, int> & aPos,
            const math::Size<3, int> & aSize,
            bool aEdges,
            bool aVertices
            );
    void pushToFrontier(
            int aGenerationIndex,
            const math::Position<3, int> & aPos,
            std::vector<int> & aGenerations,
            std::queue<std::tuple<int, math::Position<3, int>>> & aFrontier);


    int mStart;
    int mFinish;
    int mSubstrate;
    unsigned char mValue;
    bool mInertia;
    bool mLongest;
    bool mEdges;
    bool mVertices;
};

}
}
