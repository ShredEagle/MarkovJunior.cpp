#include "Node.h"
#include "Interpreter.h"
#include "AllNode.h"
#include "OneNode.h"
#include "markovjunior/ConvChain.h"
#include "markovjunior/ConvolutionNode.h"
#include "markovjunior/Map.h"
#include "markovjunior/ParallelNode.h"
#include "markovjunior/Path.h"

#include <handy/Crc.h>

namespace ad {
namespace markovjunior {

std::unique_ptr<Node> createNode(
        SequenceNode * aParent,
        const pugi::xml_node & aXmlNode,
        const SymmetryGroup & aSymmetry,
        Interpreter * aInterpreter,
        Grid * aGrid
        )
{
    switch(handy::crc64(aXmlNode.name()))
    {
        case handy::crc64("one"):
            return std::make_unique<OneNode>(aXmlNode, aSymmetry, aInterpreter, aGrid);
            break;
        case handy::crc64("all"):
            return std::make_unique<AllNode>(aXmlNode, aSymmetry, aInterpreter, aGrid);
            break;
        case handy::crc64("prl"):
            return std::make_unique<ParallelNode>(aXmlNode, aSymmetry, aInterpreter, aGrid);
            break;
        case handy::crc64("path"):
            return std::make_unique<Path>(aXmlNode, aSymmetry, aInterpreter, aGrid);
            break;
        case handy::crc64("convolution"):
            return std::make_unique<ConvolutionNode>(aXmlNode, aSymmetry, aInterpreter, aGrid);
            break;
        case handy::crc64("convchain"):
            return std::make_unique<ConvChain>(aXmlNode, aSymmetry, aInterpreter, aGrid);
            break;
        case handy::crc64("map"):
            return std::make_unique<Map>(aXmlNode, aSymmetry, aInterpreter, aGrid);
            break;
        case handy::crc64("markov"):
        case handy::crc64("sequence"):
            return createSequenceNode(
                    aParent,
                    aXmlNode,
                    aSymmetry,
                    aInterpreter,
                    aGrid);
            break;
        default:
            return std::make_unique<MarkovNode>(aParent, aXmlNode, aSymmetry, aInterpreter, aGrid);
            break;
    }
}

std::unique_ptr<SequenceNode> createSequenceNode(
        SequenceNode * aParent,
        const pugi::xml_node & aXmlNode,
        const SymmetryGroup & aSymmetry,
        Interpreter * aInterpreter,
        Grid * aGrid
        )
{
    switch(handy::crc64(aXmlNode.name()))
    {
        case handy::crc64("markov"):
            return std::make_unique<MarkovNode>(aParent, aXmlNode, aSymmetry, aInterpreter, aGrid);
            break;
        case handy::crc64("sequence"):
            return std::make_unique<SequenceNode>(aParent, aXmlNode, aSymmetry, aInterpreter, aGrid);
            break;
        default:
            return std::make_unique<MarkovNode>(aParent, aXmlNode, aSymmetry, aInterpreter, aGrid);
            break;
    }
}

std::unique_ptr<SequenceNode> createRootNode(
        const pugi::xml_node & aXmlNode,
        const SymmetryGroup & aSymmetry,
        Interpreter * aInterpreter,
        Grid * aGrid
        )
{
    std::unique_ptr<Node> baseNode = createNode(nullptr, aXmlNode, aSymmetry, aInterpreter, aGrid);

    if (!baseNode->isSequenceNode())
    {
        return std::make_unique<MarkovNode>(std::move(baseNode), aInterpreter, aGrid);
    }

    return createSequenceNode(nullptr, aXmlNode, aSymmetry, aInterpreter, aGrid);
}

bool SequenceNode::run()
{
    mInterpreter->mCurrentBranch = this;

    for (;mCurrentStep < nodes.size(); mCurrentStep++)
    {
        const std::unique_ptr<Node> & node = nodes.at(mCurrentStep); 
        if (node->run())
        {
            return true;
        }
    }

    mInterpreter->mCurrentBranch = mInterpreter->mCurrentBranch->parent;
    reset();

    return false;
}

}
}
