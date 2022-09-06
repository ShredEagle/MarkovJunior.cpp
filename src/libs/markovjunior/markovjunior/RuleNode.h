#pragma once

#include "Node.h"
#include "Interpreter.h"
#include "Rule.h"
#include "SymmetryUtils.h"

#include <math/Vector.h>

#include <pugixml.hpp>

#include <vector>

namespace ad {
namespace markovjunior {

typedef std::tuple<int, math::Position<3, int>> RuleMatch;

class RuleNode : public Node
{
public:
    RuleNode(const pugi::xml_node & aXmlNode, const SymmetryGroup & aParentSymmetry, Interpreter * aInterpreter);
    bool run() override;

    void reset() override
    {
        mLastMatchedTurn = -1;
        mCounter = 0;
        
        std::fill(mLast.begin(), mLast.end(), false);
    }
private:
    void addRule(const pugi::xml_node & aXmlNode, const SymmetryGroup & aSymmetryGroup )
    {
        Rule rule{aXmlNode, mInterpreter->mGrid, mInterpreter->mGrid};
        rule.mOriginal = true;

        SymmetryGroup ruleSymmetry = getSymmetry(
                aXmlNode.attribute("symmetry").as_string(""),
                aSymmetryGroup);

        //TODO maybe check symmetry is ok
        std::vector<Rule> symmetricRules = createAllSquareSymmetries(rule, ruleSymmetry);
        mRules.insert(mRules.end(), symmetricRules.begin(), symmetricRules.end());
    }

    void addMatch(const int ruleIndex, const math::Position<3, int> & aMatchPosition, std::vector<bool> & aMatchMask)
    {
        aMatchMask.at(mInterpreter->mGrid.getFlatGridIndex(aMatchPosition)) = true;

        if (mMatchCount < mMatches.size())
        {
            mMatches.at(mMatchCount) = {ruleIndex, aMatchPosition};
        }
        else
        {
            mMatches.emplace_back(ruleIndex, aMatchPosition);
        }
        mMatchCount++;
    }

public: 
    int mLastMatchedTurn;
    int mCounter = 0;
    int mSteps = 0;
    int mMatchCount = 0;
    std::vector<Rule> mRules;
    //bool mFutureComputed; //This is for observation
    std::vector<bool> mLast;
    std::vector<RuleMatch> mMatches;
    std::vector<std::vector<bool>> mMatchMask;

    friend std::ostream & operator<<(std::ostream & os, const RuleNode & aRuleNode);
};

}
}
