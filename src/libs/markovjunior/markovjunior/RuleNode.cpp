#include "RuleNode.h"
#include <math/Vector.h>

namespace ad {
namespace markovjunior {
RuleNode::RuleNode(const pugi::xml_node & aXmlNode, const SymmetryGroup & aParentSymmetry, Interpreter * aInterpreter ) :
    Node(aInterpreter)
{
        std::string symmetryString = aXmlNode.attribute("symmetry").as_string("");
        SymmetryGroup symmetrySubgroup = getSymmetry(symmetryString, aParentSymmetry);

        pugi::xpath_node_set xmlRuleNodes = aXmlNode.select_nodes("/rule");

        if (xmlRuleNodes.empty())
        {
            addRule(aXmlNode, symmetrySubgroup);
        }
        else
        {
            for (pugi::xpath_node node : xmlRuleNodes)
            {
                addRule(node.node(), symmetrySubgroup);
            }
        }

        mLast.assign(mRules.size(), false);
        mSteps = aXmlNode.attribute("steps").as_int();
}

bool RuleNode::run()
{
    std::fill(mLast.begin(), mLast.end(), false);

    Grid & grid = mInterpreter->mGrid;
    math::Size<3, int> gridSize = grid.mSize;

    if (mSteps > 0 && mCounter >= mSteps)
    {
        return false;
    }

    if (mLastMatchedTurn >= 0)
    {

        for (int i = mInterpreter->mFirst.at(mLastMatchedTurn); i < mInterpreter->mChanges.size(); i++)
        {
            math::Position<3, int> changePos = mInterpreter->mChanges.at(i);
            
            char value = grid.mState.at(grid.getFlatGridIndex(changePos));

            for (int ruleIndex = 0; ruleIndex < mRules.size(); ruleIndex++)
            {
                Rule rule = mRules.at(ruleIndex);
                std::vector<bool> & ruleMask = mMatchMask.at(ruleIndex);
                std::vector<math::Position<3, int>> shifts = rule.mInputShifts.at(value);

                for (math::Position<3, int> shiftPos : shifts)
                {
                    math::Position<3, int> matchPos = static_cast<math::Position<3, int>>(changePos - shiftPos);
                    
                    if (
                        matchPos.x() < 0 || matchPos.y() < 0 || matchPos.z() < 0 ||
                        matchPos.x() + rule.mInputSize.width() > gridSize.width() ||
                        matchPos.y() + rule.mInputSize.height() > gridSize.height() ||
                        matchPos.z() + rule.mInputSize.depth() > gridSize.depth()
                       )
                    {
                        continue;
                    }

                    int shiftIndex = getFlatIndex(static_cast<math::Position<3, int>>(matchPos), gridSize);

                    if (!ruleMask.at(shiftIndex) && grid.matchPatternAtPosition(rule, matchPos))
                    {
                        addMatch(ruleIndex, matchPos, ruleMask);
                    }
                }
            }
        }
    }
    else
    {
        mMatchCount = 0;

        for (int ruleIndex = 0; ruleIndex < mRules.size(); ruleIndex++)
        {
            Rule rule = mRules.at(ruleIndex);
            std::vector<bool> & ruleMask = mMatchMask.at(ruleIndex);

            for (int z = rule.mInputSize.depth() - 1; z < gridSize.depth(); z += rule.mInputSize.depth())
            {
                for (int y = rule.mInputSize.height() - 1; y < gridSize.height(); y += rule.mInputSize.height())
                {
                    for (int x = rule.mInputSize.width() - 1; x < gridSize.width(); x += rule.mInputSize.width())
                    {
                        std::vector<math::Position<3, int>> shifts = rule.mInputShifts.at(grid.mState.at(getFlatIndex({x, y, z}, gridSize)));
                        
                        for (math::Position<3, int> maskPos : shifts)
                        {
                            auto matchPos = static_cast<math::Position<3, int>>(math::Position<3, int>{x, y, z} - maskPos);

                            if (
                                matchPos.x() < 0 || matchPos.y() < 0 || matchPos.z() < 0 ||
                                matchPos.x() + rule.mInputSize.width() > gridSize.width() ||
                                matchPos.y() + rule.mInputSize.height() > gridSize.height() ||
                                matchPos.z() + rule.mInputSize.depth() > gridSize.depth()
                               )
                            {
                                continue;
                            }


                            if (grid.matchPatternAtPosition(rule, matchPos))
                            {
                                addMatch(ruleIndex, matchPos, ruleMask);
                            }
                        }
                    }
                }
            }
        }
    }

    return true;
}

std::ostream & operator<<(std::ostream & os, const RuleNode & aRuleNode)
{
    int ruleIndex = 0;
    for (auto rule : aRuleNode.mRules)
    {
        os << ruleIndex++ << " : " << std::endl << rule;
    }

    os << "RULE MATCHES" << std::endl;
    for (auto ruleMatch : aRuleNode.mMatches)
    {
        os << std::get<0>(ruleMatch) << " : " << std::get<1>(ruleMatch) << std::endl;
    }

    return os;
}
}
}
