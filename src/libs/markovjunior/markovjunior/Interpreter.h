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

enum class TestResult
{
    NOT_TESTED,
    VALID,
    NOT_VALID,
};


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
                const filesystem::path & aRelativePath,
                std::shared_ptr<std::istream> aSteam,
                const math::Size<3, int> & aSize,
                const int aSeed
                );

        void setup();
        void runStep();
        bool reloadFile();

        std::tuple<bool, bool, bool> showDebuggingTools();

        // TODO: in the future this should not exist
        const resource::ResourceLocator mResourceLocator;
        const filesystem::path mFilename;
        pugi::xml_document mXmlParsedDoc;
        SymmetryGroup mGlobalSymmetryGroup;
        math::Size<3, int> mSize;
        Grid mGrid;
        Grid mStartGrid;

        std::unique_ptr<SequenceNode> mRoot;
        SequenceNode * mCurrentBranch;
        std::vector<int> mFirst;
        std::vector<math::Position<3, int>> mChanges;
        int mCounter = 0;
        std::mt19937 mRandom;
        std::uniform_real_distribution<double> mProbabilityDistribution = std::uniform_real_distribution(0.0, 1.0);

        bool mTrackActiveRule = false;

        // Debug data
        void testFileOnMultipleSeed();
        void runTestSuite();
        void loadTest();
        std::vector<std::pair<unsigned int, TestResult>> mTestSuite;
        int mTestSuiteIndex = -1;
        bool mRunningTestSuite = 0;
        unsigned int mSeed;
        bool mRun = false;
        bool mRunningTest = false;
        std::vector<std::pair<unsigned int, bool>> mTestedSeed;
        int mStepPerTestIteration = 10;
        std::string mAcceptableValues = "";

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
