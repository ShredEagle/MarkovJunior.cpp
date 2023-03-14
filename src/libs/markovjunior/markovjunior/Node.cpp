#include "Node.h"

#include "AllNode.h"
#include "Interpreter.h"
#include "markovjunior/ConvChain.h"
#include "markovjunior/ConvolutionNode.h"
#include "markovjunior/Map.h"
#include "markovjunior/ParallelNode.h"
#include "markovjunior/Path.h"
#include "markovjunior/SymmetryUtils.h"
#include "OneNode.h"

#include <handy/Crc.h>
#include <imgui.h>

namespace ad {
namespace markovjunior {

class Grid;

std::unique_ptr<Node> createNode(SequenceNode * aParent,
                                 const pugi::xml_node & aXmlNode,
                                 const SymmetryGroup & aSymmetry,
                                 Interpreter * aInterpreter,
                                 Grid * aGrid)
{
    switch (handy::crc64(aXmlNode.name()))
    {
    case handy::crc64("one"):
        return std::make_unique<OneNode>(aXmlNode, aSymmetry, aInterpreter,
                                         aGrid);
        break;
    case handy::crc64("all"):
        return std::make_unique<AllNode>(aXmlNode, aSymmetry, aInterpreter,
                                         aGrid);
        break;
    case handy::crc64("prl"):
        return std::make_unique<ParallelNode>(aXmlNode, aSymmetry, aInterpreter,
                                              aGrid);
        break;
    case handy::crc64("path"):
        return std::make_unique<Path>(aXmlNode, aSymmetry, aInterpreter, aGrid);
        break;
    case handy::crc64("convolution"):
        return std::make_unique<ConvolutionNode>(aXmlNode, aSymmetry,
                                                 aInterpreter, aGrid);
        break;
    case handy::crc64("convchain"):
        return std::make_unique<ConvChain>(aXmlNode, aSymmetry, aInterpreter,
                                           aGrid);
        break;
    case handy::crc64("map"):
        return std::make_unique<Map>(aXmlNode, aSymmetry, aInterpreter, aGrid);
        break;
    case handy::crc64("markov"):
    case handy::crc64("sequence"):
        return createSequenceNode(aParent, aXmlNode, aSymmetry, aInterpreter,
                                  aGrid);
        break;
    default:
        return std::make_unique<MarkovNode>(aParent, aXmlNode, aSymmetry,
                                            aInterpreter, aGrid);
        break;
    }
}

std::unique_ptr<SequenceNode>
createSequenceNode(SequenceNode * aParent,
                   const pugi::xml_node & aXmlNode,
                   const SymmetryGroup & aSymmetry,
                   Interpreter * aInterpreter,
                   Grid * aGrid)
{
    switch (handy::crc64(aXmlNode.name()))
    {
    case handy::crc64("markov"):
        return std::make_unique<MarkovNode>(aParent, aXmlNode, aSymmetry,
                                            aInterpreter, aGrid);
        break;
    case handy::crc64("sequence"):
        return std::make_unique<SequenceNode>(aParent, aXmlNode, aSymmetry,
                                              aInterpreter, aGrid);
        break;
    default:
        return std::make_unique<MarkovNode>(aParent, aXmlNode, aSymmetry,
                                            aInterpreter, aGrid);
        break;
    }
}

std::unique_ptr<SequenceNode> createRootNode(const pugi::xml_node & aXmlNode,
                                             const SymmetryGroup & aSymmetry,
                                             Interpreter * aInterpreter,
                                             Grid * aGrid)
{
    std::unique_ptr<Node> baseNode =
        createNode(nullptr, aXmlNode, aSymmetry, aInterpreter, aGrid);

    if (!baseNode->isSequenceNode())
    {
        return std::make_unique<MarkovNode>(std::move(baseNode), aInterpreter,
                                            aGrid);
    }

    return createSequenceNode(nullptr, aXmlNode, aSymmetry, aInterpreter,
                              aGrid);
}

bool SequenceNode::run()
{
    mInterpreter->mCurrentBranch = this;
    mRunning = true;

    for (; mCurrentStep < nodes.size(); mCurrentStep++)
    {
        const std::unique_ptr<Node> & node = nodes.at(mCurrentStep);
        if (node->run())
        {
            if (node->mBreakOnStep)
            {
                mInterpreter->mRun = false;
            }
            return true;
        }
    }

    mInterpreter->mCurrentBranch = mInterpreter->mCurrentBranch->parent;
    reset();

    mRunning = false;
    return false;
}

void SequenceNode::debugRender(int id)
{
    const ImVec4 color = 
            mInterpreter->mCurrentBranch == this ?
            ImVec4{0.3f, 1.f, 0.3f, 1.f} :
            ImVec4{1.f, 1.f, 1.f, 1.f};
    ImGui::TextColored(color, "sequence");
    ImGui::TreePush();

    int idChild = 0;
    for (auto & node : nodes)
    {
        node->debugRender(idChild++);
    }

    ImGui::TreePop();
}

void MarkovNode::debugRender(int id)
{
    const ImVec4 color = 
            mInterpreter->mCurrentBranch == this ?
            ImVec4{0.3f, 1.f, 0.3f, 1.f} :
            ImVec4{1.f, 1.f, 1.f, 1.f};
    ImGui::TextColored(color, "markov");
    ImGui::TreePush();
    int idChild = 0;
    for (auto & node : nodes)
    {
        node->debugRender(idChild++);
    }
    ImGui::TreePop();
}

} // namespace markovjunior
} // namespace ad
