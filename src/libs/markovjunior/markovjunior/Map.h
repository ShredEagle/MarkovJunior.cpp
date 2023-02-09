#pragma once

#include "markovjunior/Rule.h"
#include "Node.h"

#include <imgui.h>
#include <math/Vector.h>
#include <pugixml.hpp>

namespace ad {
namespace markovjunior {


struct Scale
{
    math::Vec<3, int> mNumerator = math::Vec<3, int>::Zero();
    math::Vec<3, int> mDenominator = math::Vec<3, int>::Zero();
};

Grid createNewMapGrid();

Scale createMapScale(const pugi::xml_node & aXmlNode);

class Map : public SequenceNode
{
  public:
    Map(const pugi::xml_node & aXmlNode,
        const SymmetryGroup & aSymmetryGroup,
        Interpreter * aInterpreter,
        Grid * aGrid);

    static bool matches(const Rule & aRule,
                        const math::Position<3, int> & aPos,
                        const std::vector<unsigned char> & aState,
                        const math::Size<3, int> & aSize);

    static void apply(const Rule & aRule,
                      const math::Position<3, int> & aPos,
                      std::vector<unsigned char> & aState,
                      const math::Size<3, int> & aSize);

    bool run() override;

    void reset() override
    {
        SequenceNode::reset();
        mCurrentStep = -1;
    }

    void debugRender(int id = 0) override { ImGui::Text("map"); }

    Scale mScale;
    Grid mNewGrid;
    std::vector<Rule> mRules;
};

} // namespace markovjunior
} // namespace ad
