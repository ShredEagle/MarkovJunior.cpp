#pragma once
#include "SymmetryUtils.h"
#include "markovjunior/Grid.h"

#include <handy/Crc.h>
#include <imgui.h>
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
    Node(Interpreter * aInterpreter, Grid * aGrid) :
        mInterpreter{aInterpreter},
        mGrid{aGrid}
    {}

    Interpreter * mInterpreter = nullptr;
    Grid * mGrid = nullptr;

    virtual bool isRuleNode() { return false; };

    virtual bool isSequenceNode()
    {
        return false;
    }
    virtual bool run() = 0;
    virtual void reset() = 0;

    virtual void debugRender() = 0;
};

class SequenceNode;
class MarkovNode;

std::unique_ptr<Node> createNode(
        SequenceNode * aParent,
        const pugi::xml_node & aXmlNode,
        const SymmetryGroup & aSymmetry,
        Interpreter * aInterpreter,
        Grid * aGrid
        );

std::unique_ptr<SequenceNode> createSequenceNode(
        SequenceNode * aParent,
        const pugi::xml_node & aXmlNode,
        const SymmetryGroup & aSymmetry,
        Interpreter * aInterpreter,
        Grid * aGrid
        );

std::unique_ptr<SequenceNode> createRootNode(
        const pugi::xml_node & aXmlNode,
        const SymmetryGroup & aSymmetry,
        Interpreter * aInterpreter,
        Grid * aGrid
        );

class SequenceNode : public Node
{
public:
    SequenceNode * parent = nullptr;
    std::vector<std::unique_ptr<Node>> nodes;
    unsigned int mCurrentStep = 0;

    SequenceNode() = default;

    SequenceNode(Interpreter * aInterpreter, Grid * aGrid) :
        Node(aInterpreter, aGrid)
    {};

    SequenceNode(std::unique_ptr<Node> && child, Interpreter * aInterpreter, Grid * aGrid) :
        Node(aInterpreter, aGrid)
    {
        nodes.push_back(std::move(child));
    }

    SequenceNode(
            SequenceNode * aParent,
            const pugi::xml_node & aXmlNode,
            const SymmetryGroup & parentSymmetry,
            Interpreter * aInterpreter,
            Grid * aGrid
            ) :
        parent{aParent}
    {
        setupSequenceNode(this,
            aXmlNode,
            parentSymmetry,
            aInterpreter,
            aGrid);
    }

    bool isSequenceNode() override
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

        mCurrentStep = 0;
    }

    void debugRender() override;

protected:
    static void setupSequenceNode(SequenceNode * aSequenceNode,
            const pugi::xml_node & aXmlNode,
            const SymmetryGroup & parentSymmetry,
            Interpreter * aInterpreter,
            Grid * aGrid
            )
    {
        aSequenceNode->mInterpreter = aInterpreter;
        aSequenceNode->mGrid = aGrid;
        std::string symmetryString = aXmlNode.attribute("symmetry").as_string("");
        //assert(!symmetryString.empty()); Why did I put that here
        SymmetryGroup symmetrySubgroup = getSymmetry(symmetryString, parentSymmetry);

        for (auto child : aXmlNode.children())
        {
            if (handy::crc64(child.name()) != handy::crc64("union"))
            {
                aSequenceNode->nodes.push_back(createNode(aSequenceNode, child, symmetrySubgroup, aInterpreter, aGrid));

                if (aSequenceNode->nodes.back()->isSequenceNode())
                {
                    dynamic_cast<SequenceNode*>(aSequenceNode->nodes.back().get())->parent = aSequenceNode;
                }
            }
        }
    }
};

class MarkovNode : public SequenceNode
{
public:
    using SequenceNode::SequenceNode;

    MarkovNode(std::unique_ptr<Node> && aChild, Interpreter * aInterpreter, Grid * aGrid) :
        SequenceNode(std::move(aChild), aInterpreter, aGrid)
    {}

    bool run() override
    {
        mCurrentStep = 0;
        return SequenceNode::run();
    }

    void debugRender() override
    {
        ImGui::Text("Markov");
        ImGui::TreePush();
        for (auto & node : nodes)
        {
            node->debugRender();
        }
        ImGui::TreePop();
    }
};
}
}
