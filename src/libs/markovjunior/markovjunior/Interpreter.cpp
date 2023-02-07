#include "Interpreter.h"

namespace ad {
namespace markovjunior {

Interpreter::Interpreter(
        const filesystem::path & aAssetRoot,
        std::shared_ptr<std::istream> aStream,
        const math::Size<3, int> & aSize,
        const int aSeed) :
    mResourceLocator(aAssetRoot),
    mXmlParsedDoc{streamToXmlParsedDoc(*aStream)},
    mGlobalSymmetryGroup(
            getSymmetry(
                mXmlParsedDoc.attribute("symmetry").as_string(""))),
    mGrid{
        mXmlParsedDoc.document_element(),
        aSize
    },
    mStartGrid{mGrid},
    mRoot{
        createRootNode(
                mXmlParsedDoc.document_element(), mGlobalSymmetryGroup, this, &this->mGrid)
    }, 
    mCurrentBranch{mRoot.get()},
    mRandom(aSeed),
    mOrigin{mXmlParsedDoc.document_element().attribute("origin").as_bool(false)}
{
}

Interpreter::Interpreter(
        const filesystem::path & aAssetRoot,
        const filesystem::path & aRelativePath,
        const math::Size<3, int> & aSize,
        const int aSeed
        ) :
    Interpreter(
            aAssetRoot,
            std::make_shared<std::ifstream>(aAssetRoot / aRelativePath),
            aSize,
            aSeed
            )
{
    // FIX: Check size is not 0
}

void Interpreter::setup()
{
    mGrid.clear();
    if (mOrigin)
    {
        mGrid.mState.at(mGrid.getFlatGridIndex({mGrid.mSize.width() / 2, mGrid.mSize.height() / 2, mGrid.mSize.depth() / 2})) = 1;
    }
    
    mChanges.clear();
    mFirst.clear();
    mFirst.push_back(0);

    mRoot->reset();
    mCurrentBranch = mRoot.get();
}

void Interpreter::runStep()
{
    mCurrentBranch->run();
    mCounter++;
    mFirst.push_back(static_cast<int>(mChanges.size()));
}
}
}
