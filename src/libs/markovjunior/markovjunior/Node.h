#pragma once
#include "SymmetryUtils.h"

#include <pugixml.hpp>

#include <memory>
#include <cassert>

namespace ad {
namespace markovjunior {

class Interpreter;

class Node
{
public:
    virtual ~Node() = default;

    Node() = default;
    Node(Interpreter * aInterpreter) :
        mInterpreter{aInterpreter}
    {}

    Interpreter * mInterpreter = nullptr;

    virtual bool canBeRoot()
    {
        return false;
    }
    virtual bool run() = 0;
    virtual void reset() = 0;
};

class SequenceNode;
class MarkovNode;

std::unique_ptr<Node> createNode(
        SequenceNode * aParent,
        const pugi::xml_node & aXmlNode,
        const SymmetryGroup & aSymmetry,
        Interpreter * aInterpreter
        );

std::unique_ptr<SequenceNode> createSequenceNode(
        SequenceNode * aParent,
        const pugi::xml_node & aXmlNode,
        const SymmetryGroup & aSymmetry,
        Interpreter * aInterpreter
        );

std::unique_ptr<SequenceNode> createRootNode(
        const pugi::xml_node & aXmlNode,
        const SymmetryGroup & aSymmetry,
        Interpreter * aInterpreter
        );

class SequenceNode : public Node
{
public:
    SequenceNode * parent = nullptr;
    std::vector<std::unique_ptr<Node>> nodes;
    int currentStep = 0;

    SequenceNode(std::unique_ptr<Node> && child, Interpreter * aInterpreter) :
        Node(aInterpreter)
    {
        nodes.push_back(std::move(child));
    }

    SequenceNode(
            SequenceNode * aParent,
            const pugi::xml_node & aXmlNode,
            const SymmetryGroup & parentSymmetry,
            Interpreter * aInterpreter
            ) :
        Node(aInterpreter),
        parent{aParent}
    {
        std::string symmetryString = aXmlNode.attribute("symmetry").as_string("");
        //assert(!symmetryString.empty()); Why did I put that here
        SymmetryGroup symmetrySubgroup = getSymmetry(symmetryString, parentSymmetry);

        for (auto child : aXmlNode.children())
        {
            nodes.push_back(std::move(createNode(this, child, symmetrySubgroup, aInterpreter)));
        }
    }

    bool canBeRoot() override
    {
        return true;
    }

    bool run() override;

    void reset() override
    {
        for (const std::unique_ptr<Node> & node : nodes)
        {
            node->reset();
        }

        currentStep = 0;
    }

    friend std::ostream & operator<<(std::ostream & os, const SequenceNode & aNode);
};

class MarkovNode : public SequenceNode
{
public:
    using SequenceNode::SequenceNode;

    MarkovNode(std::unique_ptr<Node> && aChild, Interpreter * aInterpreter) :
        SequenceNode(std::move(aChild), aInterpreter)
    {}

    bool run() override
    {
        currentStep = 0;
        return SequenceNode::run();
    }
};
}
}
