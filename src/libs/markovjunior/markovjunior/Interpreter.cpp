#include "Interpreter.h"

#include "markovjunior/Grid.h"

#include <cstdio>
#include <fstream>
#include <imgui.h>
#include <string>
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
            if (mAcceptableValues.find(std::string(1, (char)mGrid.mCharacters.at(value))) == std::string::npos)
            {
                valid = false;
            }
        }
        mTestedSeed.emplace_back(mSeed, valid);
        mSeed += 1;
        setup();
    }
}

void Interpreter::loadTest()
{
    mSeed = mTestSuite.at(mTestSuiteIndex).first;
    setup();
}

void Interpreter::runTestSuite()
{
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
            if (mAcceptableValues.find(std::string(1, (char)mGrid.mCharacters.at(value))) == std::string::npos)
            {
                valid = false;
            }
        }

        mTestSuite.at(mTestSuiteIndex).second = valid ? TestResult::VALID : TestResult::NOT_VALID;

        mTestSuiteIndex++;

        if (mTestSuiteIndex == static_cast<int>(mTestSuite.size()))
        {
            mRun = false;
            mRunningTestSuite = false;
            mTestSuiteIndex = -1;
        }
        else
        {
            loadTest();
        }
    }
}

std::tuple<bool, bool, bool> Interpreter::showDebuggingTools()
{
    bool step = false;
    static bool onlyShowBad = false;
    static char saveTestFilePath[256] = "/home/franz/gamedev/markov_test_result";

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
        ImGui::PopItemWidth();
        ImGui::Checkbox("Track active rule", &mTrackActiveRule);
        ImGui::BeginChild("Tree debugging", ImVec2(0, 0), true);
        mRoot->debugRender();
        ImGui::EndChild();
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Test xml"))
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
        ImGui::Checkbox("Show bad", &onlyShowBad);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100.f);
        ImGui::DragInt("Steps per iteration", &mStepPerTestIteration, 1, -1, 1000);
        if (ImGui::Button("Save results"))
        {
            std::ofstream fileStream;
            fileStream.open(saveTestFilePath);
            if (fileStream.is_open())
            {
                for (auto [seed, valid] : mTestedSeed)
                {
                    if (!valid)
                    {
                        fileStream << seed << std::endl;
                    }
                }
            }
            fileStream.close();
        }
        ImGui::SameLine();
        ImGui::InputText("Result path", saveTestFilePath, IM_ARRAYSIZE(saveTestFilePath));

        ImGui::BeginChild("Results", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);
        int good = 0;
        int bad = 0;
        for (auto [seed, valid] : mTestedSeed)
        {
            if (valid)
            {
                good++;
            }
            else
            {
                bad++;
            }

            if ((onlyShowBad && !valid) || !onlyShowBad)
            {
                ImGui::Text("%u is %s", seed, valid ? "good" : "not good");
            }
        }
        ImGui::EndChild();
        ImGui::Text("%d good / %d bad", good, bad);
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Run test file"))
    {

        if (ImGui::Button("Load file"))
        {
            mTestSuite.clear();
            std::ifstream file;
            file.open(saveTestFilePath);
            if (file.is_open())
            {
                char readSeed[16];
                while (file.getline(&readSeed[0], IM_ARRAYSIZE(readSeed)))
                {
                    unsigned int seed;
                    std::stringstream seedStream;
                    seedStream << readSeed;
                    seedStream >> seed;
                    mTestSuite.push_back({seed, TestResult::NOT_TESTED});
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Run file"))
        {
            mTestSuiteIndex = 0;
            mRunningTestSuite = true;
            mStepPerTestIteration = -1;
            reloadFile();
            loadTest();
        }

        ImGui::InputText("Suite path", saveTestFilePath, IM_ARRAYSIZE(saveTestFilePath));
        if(ImGui::BeginTable("Tests", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("Seed");
            ImGui::TableSetupColumn("Result");
            ImGui::TableSetupColumn("");
            ImGui::TableHeadersRow();
            int i = 0;
            for (auto [seed, result] : mTestSuite)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                if (i == mTestSuiteIndex)
                {
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("R");
                }
                ImGui::TableSetColumnIndex(1);
                ImGui::AlignTextToFramePadding();
                ImGui::Text("%u", seed);
                ImGui::TableSetColumnIndex(2);
                ImGui::AlignTextToFramePadding();
                switch(result)
                {
                    case TestResult::NOT_TESTED:
                        ImGui::Text("Not tested");
                        break;
                    case TestResult::VALID:
                        ImGui::Text("Valid");
                        break;
                    case TestResult::NOT_VALID:
                        ImGui::Text("bad");
                        break;
                    default:
                        break;

                }
                ImGui::TableSetColumnIndex(3);
                ImGui::PushID(std::to_string(i).c_str());
                if(ImGui::Button("Replay seed", ImVec2(-FLT_MIN, 0.f)))
                {
                    mTestSuiteIndex = i;
                    loadTest();
                }
                ImGui::PopID();
                i++;
            }
            ImGui::EndTable();
        }
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
    ImGui::End();

    return std::tie(mRun, step, mRunningTest);
}
} // namespace markovjunior
} // namespace ad
