#include "RuleNode.h"

#include "Field.h"
#include "GridUtils.h"
#include "Observation.h"
#include "Search.h"
#include "markovjunior/Interpreter.h"    // for Interpreter
#include "markovjunior/Node.h"           // for Node
#include "markovjunior/Rule.h"           // for Rule, countTrailingZeroes
#include "markovjunior/SymmetryUtils.h"  // for getSymmetry, SymmetryGroup

#include <math/Color.h>
#include <math/Vector.h>

#include <algorithm>
#include <cstddef>
#include <imgui.h>

#include <iostream>                      // for operator<<, endl, ostream
#include <iterator>                      // for distance
#include <map>                           // for map
#include <random>                        // for mt19937
#include <string>                        // for string, char_traits
#include <utility>                       // for __tuple_element_t

namespace ad {
namespace markovjunior {
RuleNode::RuleNode(const pugi::xml_node & aXmlNode,
                   const SymmetryGroup & aParentSymmetry,
                   Interpreter * aInterpreter,
                   Grid * aGrid) :
    Node(aInterpreter, aGrid)
{
    std::string symmetryString = aXmlNode.attribute("symmetry").as_string("");
    SymmetryGroup symmetrySubgroup =
        getSymmetry(symmetryString, aParentSymmetry);

    pugi::xpath_node_set xmlRuleNodes = aXmlNode.select_nodes("rule");

    if (xmlRuleNodes.empty())
    {
        addRule(aXmlNode, symmetrySubgroup);
    }
    else
    {
        for (const pugi::xpath_node & node : xmlRuleNodes)
        {
            addRule(node.node(), symmetrySubgroup);
        }
    }

    mLast.assign(mRules.size(), false);
    mSteps = aXmlNode.attribute("steps").as_int();

    const Grid & grid = *mGrid;

    mTemperature = aXmlNode.attribute("temperature").as_double(0.);
    pugi::xpath_node_set xmlFieldNodes = aXmlNode.select_nodes("field");

    if (!xmlFieldNodes.empty())
    {
        mFields = std::vector<Field>(grid.mCharacters.size(), Field{});
        for (const pugi::xpath_node & node : xmlFieldNodes)
        {
            unsigned char character =
                node.node().attribute("for").as_string()[0];
            if (grid.mValues.contains(character))
            {
                mFields.at(grid.mValues.at(character)) =
                    Field(node.node(), grid);
            }
            else
            {
                std::cout << "UNKNOWN FIELD VALUE" << std::endl;
            }
        }
        mPotentials = std::vector<std::vector<int>>(
            grid.mCharacters.size(), std::vector<int>(grid.mState.size(), 0));
    }

    pugi::xpath_node_set xmlObservationsNode = aXmlNode.select_nodes("observe");

    if (!xmlObservationsNode.empty())
    {
        mObservations =
            std::vector<Observation>(grid.mCharacters.size(), Observation{});

        for (const pugi::xpath_node & node : xmlObservationsNode)
        {
            unsigned char value =
                grid.mValues.at(node.node().attribute("value").as_string()[0]);
            std::string character(
                1, static_cast<char>(grid.mCharacters.at(value)));
            mObservations.at(value) = Observation(
                node.node().attribute("from").as_string(character.c_str())[0],
                node.node().attribute("to").as_string(), grid);
        }

        mSearch = aXmlNode.attribute("search").as_bool(false);
        if (mSearch)
        {
            mLimit = aXmlNode.attribute("limit").as_int(-1);
            mDepthCoefficient =
                aXmlNode.attribute("depthCoefficient").as_float(0.5);
        }
        else
        {
            mPotentials = std::vector<std::vector<int>>(
                grid.mCharacters.size(),
                std::vector<int>(grid.mState.size(), 0));
        }
        mFuture = std::vector<int>(grid.mState.size(), 0);
    }
}

bool RuleNode::run()
{
    std::fill(mLast.begin(), mLast.end(), false);

    Grid & grid = *mGrid;
    math::Size<3, int> gridSize = grid.mSize;

    if (mSteps > 0 && mCounter >= mSteps)
    {
        return false;
    }

    if (!mObservations.empty() && !mFutureComputed)
    {
        if (!Observation::computeFutureSetPresent(mFuture, grid.mState,
                                                  mObservations))
        {
            return false;
        }
        else
        {
            mFutureComputed = true;
            if (mSearch)
            {
                int maxTries = mLimit < 0 ? 1 : 20;
                for (int k = 0; k < maxTries && mTrajectory.size() == 0; k++)
                {
                    mTrajectory =
                        runSearch(grid.mState, mFuture, mRules, grid.mSize,
                                  static_cast<int>(grid.mCharacters.size()),
                                  mAllSearch, mLimit, mDepthCoefficient,
                                  static_cast<int>(mInterpreter->mRandom()));
                }

                if (mTrajectory.size() == 0)
                {
                    return false;
                }
            }
            else
            {
                Observation::computeBackwardPotentials(mPotentials, mFuture,
                                                       grid.mSize, mRules);
            }
        }
    }

    if (mLastMatchedTurn >= 0)
    {
        for (unsigned int i = mInterpreter->mFirst.at(mLastMatchedTurn);
             i < mInterpreter->mChanges.size(); i++)
        {
            math::Position<3, int> changePos = mInterpreter->mChanges.at(i);

            unsigned char value =
                grid.mState.at(grid.getFlatGridIndex(changePos));

            for (unsigned int ruleIndex = 0; ruleIndex < mRules.size();
                 ruleIndex++)
            {
                Rule rule = mRules.at(ruleIndex);
                std::vector<bool> & ruleMask = mMatchMask.at(ruleIndex);
                std::vector<math::Vec<3, int>> shifts =
                    rule.mInputShifts.at(value);

                for (math::Vec<3, int> shiftPos : shifts)
                {
                    math::Position<3, int> matchPos = changePos - shiftPos;

                    if (matchPos.x() < 0 || matchPos.y() < 0 || matchPos.z() < 0
                        || matchPos.x() + rule.mInputSize.width()
                               > gridSize.width()
                        || matchPos.y() + rule.mInputSize.height()
                               > gridSize.height()
                        || matchPos.z() + rule.mInputSize.depth()
                               > gridSize.depth())
                    {
                        continue;
                    }

                    int shiftIndex = getFlatIndex(
                        static_cast<math::Position<3, int>>(matchPos),
                        gridSize);

                    if (!ruleMask.at(shiftIndex)
                        && grid.matchPatternAtPosition(rule, matchPos))
                    {
                        addMatch(static_cast<int>(ruleIndex), matchPos,
                                 ruleMask);
                    }
                }
            }
        }
    }
    else
    {
        mMatchCount = 0;

        for (unsigned int ruleIndex = 0; ruleIndex < mRules.size(); ruleIndex++)
        {
            Rule rule = mRules.at(ruleIndex);
            std::vector<bool> & ruleMask = mMatchMask.at(ruleIndex);

            for (int z = rule.mInputSize.depth() - 1; z < gridSize.depth();
                 z += rule.mInputSize.depth())
            {
                for (int y = rule.mInputSize.height() - 1;
                     y < gridSize.height(); y += rule.mInputSize.height())
                {
                    for (int x = rule.mInputSize.width() - 1;
                         x < gridSize.width(); x += rule.mInputSize.width())
                    {
                        std::vector<math::Vec<3, int>> shifts =
                            rule.mInputShifts.at(grid.mState.at(
                                getFlatIndex({x, y, z}, gridSize)));

                        for (math::Vec<3, int> maskPos : shifts)
                        {
                            auto matchPos =
                                math::Position<3, int>{x, y, z} - maskPos;

                            if (matchPos.x() < 0 || matchPos.y() < 0
                                || matchPos.z() < 0
                                || matchPos.x() + rule.mInputSize.width()
                                       > gridSize.width()
                                || matchPos.y() + rule.mInputSize.height()
                                       > gridSize.height()
                                || matchPos.z() + rule.mInputSize.depth()
                                       > gridSize.depth())
                            {
                                continue;
                            }

                            if (grid.matchPatternAtPosition(rule, matchPos))
                            {
                                addMatch(static_cast<int>(ruleIndex), matchPos,
                                         ruleMask);
                            }
                        }
                    }
                }
            }
        }
    }

    if (!mFields.empty())
    {
        bool anySuccess = false;
        bool anyComputation = false;

        for (unsigned int i = 0; i != mFields.size(); i++)
        {
            auto field = mFields.at(i);

            if (field.mSubstrate != -1 && (mCounter == 0 || field.mRecompute))
            {
                bool success = field.compute(mPotentials.at(i), grid);

                if (!success && field.mEssential)
                {
                    return false;
                }

                anySuccess |= success;
                anyComputation = true;
            }
        }

        return !anyComputation || anySuccess;
    }

    return true;
}

constexpr float cellSize = 10.f;
constexpr float cellSpacing = 2.f;
constexpr float ruleSpacing = 6.f;
constexpr float inputOutputSpacing = 15.f;
static const ImU32 imguiWhite = ImColor(ImVec4{1.f, 1.f, 1.f, 1.f});

void RuleNode::debugRender(int id)
{
    ImDrawList * drawList = ImGui::GetWindowDrawList();
    const ImU32 imguiRed = ImColor(ImVec4{1.f, 0.3f, 0.3f, 1.f});
    const ImU32 imguiYellow = ImColor(ImVec4{1.f, 1.f, 0.3f, 1.f});

    const ImVec2 p = ImGui::GetCursorScreenPos();
    const float localY = ImGui::GetCursorPosY();
    float posX = p.x;
    float posY = p.y + 2.f;

    for (auto ruleIt = mRules.begin(); ruleIt != mRules.end(); ruleIt++)
    {
        const Rule & rule = *ruleIt;

        if (!rule.mOriginal)
        {
            continue;
        }

        float startRuleY = posY;
        float startInputX = p.x;

        for (std::size_t y = 0; y < (std::size_t)rule.mInputSize.height(); ++y)
        {
            posX = startInputX;
            for (std::size_t x = 0; x < (std::size_t)rule.mInputSize.width(); ++x)
            {
                unsigned char ruleChar = rule.mByteInput.at(getFlatIndex({static_cast<int>(x), static_cast<int>(y), 0}, rule.mInputSize));
                auto colorHdr = getColorfromValue(*mGrid, ruleChar);
                drawImGuiColorSquare(drawList, colorHdr, posX, posY);

                posX += cellSize + cellSpacing;
            }
            posY += cellSize + cellSpacing;
        }

        posY = startRuleY;
        posX += inputOutputSpacing;
        float startOutputX = posX;

        for (std::size_t y = 0; y < (std::size_t)rule.mOutputSize.height(); ++y)
        {
            posX = startOutputX;
            for (std::size_t x = 0; x < (std::size_t)rule.mOutputSize.width(); ++x)
            {
                unsigned char ruleChar = rule.mOutputs.at(getFlatIndex({static_cast<int>(x), static_cast<int>(y), 0}, rule.mOutputSize));
                auto colorHdr = getColorfromValue(*mGrid, ruleChar);
                drawImGuiColorSquare(drawList, colorHdr, posX, posY);
                posX += cellSize + cellSpacing;
            }
            posY += cellSize + cellSpacing;
        }

        for (auto allSimilarRuleIt = ruleIt; allSimilarRuleIt != mRules.end();
             allSimilarRuleIt++)
        {
            if (allSimilarRuleIt->mOriginal && allSimilarRuleIt != ruleIt)
            {
                break;
            }
            bool isActive =
                mLast.at(std::distance(mRules.begin(), allSimilarRuleIt))
                || mTrajectory.size() > mCounter;
            if (mRunning && isActive)
            {
                drawList->AddTriangleFilled(
                    ImVec2(posX + ruleSpacing, startRuleY + 5.f),
                    ImVec2(posX + ruleSpacing + cellSize, startRuleY),
                    ImVec2(posX + ruleSpacing + cellSize, startRuleY + 10.f),
                    imguiYellow);
                if (mInterpreter->mTrackActiveRule)
                {
                    ImGui::SetScrollY(localY);
                }
            }
        }

        posY += ruleSpacing;
    }

    ImGui::Dummy(ImVec2(posX - p.x, posY - p.y));
    /* if (ImGui::IsItemHovered()) */
    /* { */
    /*     ImGui::BeginTooltip(); */
    /*     std::string content; */
    /*     for (bool lastMatch : mLast) */
    /*     { */
    /*         content += lastMatch ? "true, " : "false, "; */
    /*     } */
    /*     content = content.substr(0, content.size() - 2); */
    /*     ImGui::Text("mLast: %s", content.c_str()); */
    /*     ImGui::EndTooltip(); */
    /* } */

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        mBreakOnStep = !mBreakOnStep;
    }

    if (mObservations.size() > 0)
    {
        ImGui::Indent(30.f);
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Observations");
        ImGui::SameLine();
        ImGui::Checkbox("Show trajectory when available", &mShowTrajectory);
        for (const Observation & obs : mObservations)
        {
            if (obs.mTo == -1)
            {
                continue;
            }
            int to = obs.mTo;
            auto colorHdrFrom = getColorfromValue(*mGrid, obs.mFrom);

            ImGui::Text("From");
            ImGui::SameLine();
            const ImVec2 fromPos = ImGui::GetCursorScreenPos();
            drawImGuiColorSquare(drawList, colorHdrFrom, fromPos.x, fromPos.y);
            ImGui::Dummy(ImVec2(cellSize, cellSize));
            ImGui::SameLine();
            ImGui::Text("to");
            ImGui::SameLine();
            const ImVec2 toPos = ImGui::GetCursorScreenPos();
            float x = toPos.x;

            while (to != 0)
            {
                int trail = countTrailingZeroes(to);
                int wave = 1 << trail;
                to &= ~wave;

                auto colorHdrTo = getColorfromValue(*mGrid, trail);
                drawImGuiColorSquare(drawList, colorHdrTo, x, toPos.y);
                x+= cellSize + cellSpacing;
            }
            ImGui::Dummy(ImVec2(x - toPos.x, cellSize));
        }
        ImGui::Unindent(30.f);
    }

    if (mBreakOnStep)
    {
        posX += ruleSpacing;
        drawList->AddCircleFilled(
            ImVec2{posX + (2.f * ruleSpacing) + (1.5f * cellSize),
                   p.y + cellSize / 2.f + 2.f},
            cellSize / 2.f, imguiRed);
    }
}
void drawImGuiColorSquare(ImDrawList * aDrawList,
                                 math::hdr::Rgb<float> aColorHdr,
                                 float aX,
                                 float aY)
{
    const ImU32 color =
        ImColor(ImVec4{aColorHdr.r(), aColorHdr.g(), aColorHdr.b(), 1.f});
    aDrawList->AddRectFilled(ImVec2(aX, aY),
                             ImVec2(aX + cellSize, aY + cellSize), color);
    aDrawList->AddRect(ImVec2(aX, aY),
                       ImVec2(aX + cellSize, aY + cellSize), imguiWhite,
                       0.f, ImDrawFlags_None, 1.f);
}

std::ostream & operator<<(std::ostream & os, const RuleNode & aRuleNode)
{
    int ruleIndex = 0;
    for (const auto & rule : aRuleNode.mRules)
    {
        os << ruleIndex++ << " : " << std::endl << rule;
    }

    os << "RULE MATCHES" << std::endl;
    for (auto ruleMatch : aRuleNode.mMatches)
    {
        os << std::get<0>(ruleMatch) << " : " << std::get<1>(ruleMatch)
           << std::endl;
    }

    return os;
}
} // namespace markovjunior
} // namespace ad
