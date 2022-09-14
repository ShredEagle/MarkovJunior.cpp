#include "Node.h"
#include "Interpreter.h"
#include "AllNode.h"
#include "OneNode.h"
#include "markovjunior/ParallelNode.h"
#include "markovjunior/Path.h"

#include <handy/Crc.h>

namespace ad {
namespace markovjunior {

std::unique_ptr<Node> createNode(
        SequenceNode * aParent,
        const pugi::xml_node & aXmlNode,
        const SymmetryGroup & aSymmetry,
        Interpreter * aInterpreter
        )
{
    switch(handy::crc64(aXmlNode.name()))
    {
        case handy::crc64("one"):
            return std::make_unique<OneNode>(aXmlNode, aSymmetry, aInterpreter);
            break;
        case handy::crc64("all"):
            return std::make_unique<AllNode>(aXmlNode, aSymmetry, aInterpreter);
            break;
        case handy::crc64("prl"):
            return std::make_unique<ParallelNode>(aXmlNode, aSymmetry, aInterpreter);
            break;
        case handy::crc64("path"):
            return std::make_unique<Path>(aXmlNode, aSymmetry, aInterpreter);
            break;
        case handy::crc64("markov"):
        case handy::crc64("sequence"):
            return createSequenceNode(
                    aParent,
                    aXmlNode,
                    aSymmetry,
                    aInterpreter);
            break;
        default:
            return std::make_unique<MarkovNode>(aParent, aXmlNode, aSymmetry, aInterpreter);
            break;
    }
}

std::unique_ptr<SequenceNode> createSequenceNode(
        SequenceNode * aParent,
        const pugi::xml_node & aXmlNode,
        const SymmetryGroup & aSymmetry,
        Interpreter * aInterpreter
        )
{
    switch(handy::crc64(aXmlNode.name()))
    {
        case handy::crc64("markov"):
            return std::make_unique<MarkovNode>(aParent, aXmlNode, aSymmetry, aInterpreter);
            break;
        case handy::crc64("sequence"):
            return std::make_unique<SequenceNode>(aParent, aXmlNode, aSymmetry, aInterpreter);
            break;
        default:
            return std::make_unique<MarkovNode>(aParent, aXmlNode, aSymmetry, aInterpreter);
            break;
    }
}

std::unique_ptr<SequenceNode> createRootNode(
        const pugi::xml_node & aXmlNode,
        const SymmetryGroup & aSymmetry,
        Interpreter * aInterpreter
        )
{
    std::unique_ptr<Node> baseNode = createNode(nullptr, aXmlNode, aSymmetry, aInterpreter);

    if (!baseNode->canBeRoot())
    {
        return std::make_unique<MarkovNode>(std::move(baseNode), aInterpreter);
    }

    return createSequenceNode(nullptr, aXmlNode, aSymmetry, aInterpreter);
}

bool SequenceNode::run()
{
    mInterpreter->mCurrentBranch = this;

    for (;currentStep < nodes.size(); currentStep++)
    {
        const std::unique_ptr<Node> & node = nodes.at(currentStep); 
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
