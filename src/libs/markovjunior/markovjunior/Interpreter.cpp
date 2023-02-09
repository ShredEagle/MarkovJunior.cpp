#include "Interpreter.h"

#include "markovjunior/Grid.h"

#include <imgui.h>
#include <tuple>

namespace ad {
namespace markovjunior {

Interpreter::Interpreter(const filesystem::path & aAssetRoot,
        const filesystem::path & aRelativePath,
                         std::shared_ptr<std::istream> aStream,
                         const math::Size<3, int> & aSize,
                         const int aSeed) :
    mResourceLocator(aAssetRoot),
    mFilename(aRelativePath),
    mXmlParsedDoc{streamToXmlParsedDoc(*aStream)},
    mGlobalSymmetryGroup(
        getSymmetry(mXmlParsedDoc.attribute("symmetry").as_string(""))),
    mSize{(assert(aSize.width() > 0 && aSize.height() > 0 && aSize.depth() > 0
                  && "aSize can't have a 0 value in any dimension"),
           aSize)},
    mGrid{mXmlParsedDoc.document_element(), mSize},
    mStartGrid{mGrid},
    mRoot{createRootNode(mXmlParsedDoc.document_element(),
                         mGlobalSymmetryGroup,
                         this,
                         &this->mGrid)},
    mCurrentBranch{mRoot.get()},
    mRandom(aSeed),
    mSeed{static_cast<unsigned int>(aSeed)},
    mAcceptableValues{mXmlParsedDoc.document_element().attribute("acceptable").as_string("")},
    mOrigin{mXmlParsedDoc.document_element().attribute("origin").as_bool(false)}
{}

Interpreter::Interpreter(const filesystem::path & aAssetRoot,
                         const filesystem::path & aRelativePath,
                         const math::Size<3, int> & aSize,
                         const int aSeed) :
    Interpreter(aAssetRoot,
                aRelativePath,
                std::make_shared<std::ifstream>(aAssetRoot / aRelativePath),
                aSize,
                aSeed)
{}

bool Interpreter::reloadFile() {
    std::shared_ptr<std::ifstream> fileStream = std::make_shared<std::ifstream>(mResourceLocator.pathFor(mFilename));
    mXmlParsedDoc = streamToXmlParsedDoc(*fileStream);
    mGlobalSymmetryGroup = 
        getSymmetry(mXmlParsedDoc.attribute("symmetry").as_string(""));
    mGrid = Grid{mXmlParsedDoc.document_element(), mSize};
    mStartGrid = mGrid;
    mRoot = createRootNode(mXmlParsedDoc.document_element(),
                           mGlobalSymmetryGroup, this, &this->mGrid);
    mCurrentBranch = mRoot.get();

    return true;
}

void Interpreter::setup()
{
    mGrid.clear();
    if (mSize != mGrid.mSize)
    {
        mGrid = Grid{mXmlParsedDoc.document_element(), mSize};
        mStartGrid = mGrid;
        mRoot = createRootNode(mXmlParsedDoc.document_element(),
                               mGlobalSymmetryGroup, this, &this->mGrid);
        mCurrentBranch = mRoot.get();
    }
    if (mOrigin)
    {
        mGrid.mState.at(mGrid.getFlatGridIndex({mGrid.mSize.width() / 2,
                                                mGrid.mSize.height() / 2,
                                                mGrid.mSize.depth() / 2})) = 1;
    }

    mChanges.clear();
    mFirst.clear();
    mFirst.push_back(0);

    mRoot->reset();
    mCurrentBranch = mRoot.get();
    mCounter = 0;
    mRandom.seed(mSeed);
}

void Interpreter::runStep()
{
    mCurrentBranch->run();
    mCounter++;
    mFirst.push_back(static_cast<int>(mChanges.size()));
}

void Interpreter::testFileOnMultipleSeed()
{
    mRunningTest = true;
    int steps = 0;
    while(mCurrentBranch != nullptr && (mStepPerTestIteration == -1 || steps++ < mStepPerTestIteration))
    {
        runStep();
    }

    if (mCurrentBranch == nullptr)
    {
        bool valid = true;
        for (unsigned char value : mGrid.mState)
        {
            if (mAcceptableValues.find(std::string(1, mGrid.mCharacters.at(value))) == std::string::npos)
            {
                valid = false;
            }
        }
        mTestedSeed.emplace_back(mSeed, valid);
        mSeed += 1;
        setup();
    }
}

std::tuple<bool, bool, bool> Interpreter::showDebuggingTools()
{
    bool step = false;
    static bool onlyShowBad = false;

    ImGui::Begin("Interpreter debug");
    ImGui::BeginTabBar("interpreter tab bar");
    if (ImGui::BeginTabItem("Debugging"))
    {
        if (ImGui::Button("Play"))
        {
            mRun = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop"))
        {
            mRun = false;
        }
        if (!mRun)
        {
            ImGui::SameLine();
            if (ImGui::Button("Step"))
            {
                step = true;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Restart"))
        {
            mRun = true;
            setup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Reload File"))
        {
            reloadFile();
            setup();
        }
        ImGui::PushItemWidth(150.f);
        unsigned int step = 1;
        ImGui::InputScalar("seed", ImGuiDataType_U32, &mSeed, &step, &step);
        ImGui::SameLine();
        ImGui::InputInt2("size", mSize.data());
        ImGui::Checkbox("Track active rule", &mTrackActiveRule);
        ImGui::BeginChild("Tree debugging", ImVec2(0, 0), true);
        mRoot->debugRender();
        ImGui::EndChild();
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Test file"))
    {
        if (ImGui::Button("Run tests"))
        {
            mRunningTest = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop tests"))
        {
            mRunningTest = false;
        }
        ImGui::SameLine();
        ImGui::Checkbox("Only show bad", &onlyShowBad);
        ImGui::DragInt("Steps per iteration", &mStepPerTestIteration, 1, -1, 1000);

        ImGui::BeginChild("Results", ImVec2(0, 0), true);
        for (auto [seed, valid] : mTestedSeed)
        {
            if ((onlyShowBad && !valid) || !onlyShowBad)
            {
                ImGui::Text("%u is %s", seed, valid ? "good" : "not good");
            }
        }
        ImGui::EndChild();
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
    ImGui::End();

    return std::tie(mRun, step, mRunningTest);
}
} // namespace markovjunior
} // namespace ad
