#pragma once
#include "Node.h"
#include "SymmetryUtils.h"
#include "Grid.h"
#include "resource/ResourceLocator.h"

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
                const filesystem::path & aAssetRoot,
                const filesystem::path & aRelativePath,
                const math::Size<3, int> & aSize,
                const int aSeed
                );
        Interpreter(
                const filesystem::path & aAssetRoot,
                std::shared_ptr<std::istream> aSteam,
                const math::Size<3, int> & aSize,
                const int aSeed
                );

        void setup();
        void runStep();

        // TODO: in the future this should not exist
        const resource::ResourceLocator mResourceLocator;
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
        std::uniform_real_distribution<double> mProbabilityDistribution = std::uniform_real_distribution(0.0, 1.0);

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
