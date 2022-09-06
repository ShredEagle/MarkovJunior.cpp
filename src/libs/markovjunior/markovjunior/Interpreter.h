#pragma once
#include "Node.h"
#include "SymmetryUtils.h"
#include "Grid.h"

#include <platform/Filesystem.h>
#include <math/Vector.h>

#include <pugixml.hpp>

#include <cassert>
#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <vector>

namespace ad {
namespace markovjunior {

class Interpreter
{
    public:
        Interpreter(
                const filesystem::path & aPath,
                const math::Size<3, int> & aSize,
                const int aSeed
                );
        Interpreter(
                std::shared_ptr<std::istream> aSteam,
                const math::Size<3, int> & aSize,
                const int aSeed
                );

        void setup();
        void runStep();

        pugi::xml_document mXmlParsedDoc;
        SymmetryGroup mGlobalSymmetryGroup;
        Grid mGrid;
        Grid mStartGrid;

        std::unique_ptr<SequenceNode> mRoot;
        SequenceNode * mCurrentBranch;
        std::vector<int> mFirst;
        std::vector<math::Position<3, int>> mChanges;
        int mCounter = 0;
        std::mt19937 mRandom;

    private:
        static pugi::xml_document pathToXmlParsedDoc(const filesystem::path & aPath)
        {
            pugi::xml_document doc;
            doc.load_file(aPath.string().c_str());
            return doc;
        }
        static pugi::xml_document streamToXmlParsedDoc(std::istream & aStream)
        {
            pugi::xml_document doc;
            doc.load(aStream);
            return doc;
        }

        bool mOrigin = false;

};
}
}
