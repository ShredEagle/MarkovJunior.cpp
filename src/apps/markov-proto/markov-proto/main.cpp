#include "markovjunior/Grid.h"
#include "markovjunior/RuleNode.h"
#include <graphics/CameraUtilities.h>
#include <markovjunior/Interpreter.h>
#include <graphics/ApplicationGlfw.h>
#include <graphics/TrivialShaping.h>

#include <chrono>
#include <math/Color.h>
#include <math/Matrix.h>
#include <math/Rectangle.h>
#include <renderer/commons.h>
#include <string>
#include <thread>

using namespace ad::markovjunior;
using namespace ad::graphics;

constexpr ad::graphics::Size2<int> gWindowSize{500, 500};

inline ApplicationGlfw application{"Markovjunior", gWindowSize, ApplicationFlag::Window_Keep_Ratio};
inline bool runSimulation = false;
inline bool stepSimul = false;

constexpr float gViewedHeight = 500.;

const std::map<std::string, ad::math::sdr::Rgb> colorMatching = {
    {"B", ad::math::sdr::gBlack},
    {"W", ad::math::sdr::gWhite},
    {"t", ad::math::sdr::Rgb{20, 30, 20}},
    {"K", ad::math::sdr::Rgb{255, 112, 146}},
    {"O", ad::math::sdr::Rgb{255, 153, 0}},
    {"E", ad::math::sdr::Rgb{70, 200, 70}},
    {"D", ad::math::sdr::Rgb{40, 100, 40}},
    {"N", ad::math::sdr::Rgb{100, 40, 40}},
    {"R", ad::math::sdr::gRed},
    {"C", ad::math::sdr::gCyan},
    {"G", ad::math::sdr::gGreen},
    {"U", ad::math::sdr::gBlue},
    {"P", ad::math::sdr::gMagenta},
    {"Y", ad::math::sdr::gYellow},
    {"A", ad::math::sdr::Rgb{100, 100, 100}}
};

std::vector<TrivialShaping::Rectangle> renderGrid(const Grid & aGrid)
{
    std::vector<TrivialShaping::Rectangle> result;

    float cellSize = gViewedHeight / (float)aGrid.mSize.width();

    for (int z = 0; z < aGrid.mSize.depth(); z++)
    {
        for (int y = 0; y < aGrid.mSize.height(); y++)
        {
            for (int x = 0; x < aGrid.mSize.width(); x++)
            {
                unsigned char value = aGrid.mCharacters.at(aGrid.mState.at(aGrid.getFlatGridIndex({x, y, z}))); 
                std::string color = std::string(1, static_cast<char>(value));
                result.push_back({
                        {
                            {x * cellSize, y * cellSize},
                            {cellSize, cellSize}
                        },
                        colorMatching.at(color)
                });
            }
        }
    }

    return result;
}

std::vector<TrivialShaping::Rectangle> renderPotential(const std::vector<std::vector<int>> & aPotentials, const Grid & aGrid)
{
    std::vector<TrivialShaping::Rectangle> result;

    float cellSize = gViewedHeight / (float)aGrid.mSize.width();

    for (int i = 0; i != aPotentials.size(); i++)
    {
        for (int z = 0; z < aGrid.mSize.depth(); z++)
        {
            for (int y = 0; y < aGrid.mSize.height(); y++)
            {
                for (int x = 0; x < aGrid.mSize.width(); x++)
                {
                    if (!aPotentials.at(i).empty())
                    {
                        unsigned char value = aGrid.mCharacters.at(i); 
                        std::string color = std::string(1, static_cast<char>(value));
                        ad::math::sdr::Rgb colorRgb = colorMatching.at(color);
                        int flatIndex = aGrid.getFlatGridIndex({x, y, z});
                        int potential = aPotentials.at(i).at(flatIndex);
                        if (potential > 0)
                        {
                            colorRgb.r() /= potential;
                            colorRgb.g() /= potential;
                            colorRgb.b() /= potential;
                            result.push_back({
                                    {
                                        {x * cellSize, y * cellSize},
                                        {cellSize, cellSize}
                                    },
                                    colorRgb
                            });
                        }
                    }
                }
            }
        }
    }

    return result;
}

inline ad::math::Rectangle<GLfloat> getViewedRectangle(ad::math::Position<2, float> aCameraPosition, float aViewportRatio)
{
    return ad::math::Rectangle<GLfloat>{
        aCameraPosition,
        ad::math::makeSizeFromHeight(gViewedHeight, aViewportRatio)
    }.centered();
}

inline void callbackKeyboard(int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        application.getAppInterface()->requestCloseApplication();
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        runSimulation = !runSimulation;
    }
    if (key == GLFW_KEY_L && action == GLFW_PRESS)
    {
        stepSimul = true;
    }
}

int main()
{
    Interpreter interpreter(
            "/home/franz/gamedev/MarkovJunior.cpp/assets/snake_with_field.xml",
            {101, 101, 1},
            345);
    //std::cout << interpreter.mGrid << std::endl;
    interpreter.setup();
    //std::cout << interpreter.mGrid << std::endl;
    
    application.getAppInterface()->registerKeyCallback(&callbackKeyboard);

    TrivialShaping shapes{application.getAppInterface()->getWindowSize()};

    ad::math::Rectangle<GLfloat> viewed = getViewedRectangle(
            {gViewedHeight / 2., gViewedHeight / 2.},
            ad::math::getRatio<GLfloat>(
                application.getAppInterface()->getWindowSize()));

    setViewedRectangle(shapes, viewed);

    while(application.nextFrame())
    {
        application.getAppInterface()->clear();

        if (runSimulation || stepSimul)
        {
            for (int i = 0; i != 1; i++)
            {
                if (interpreter.mCurrentBranch != nullptr)
                {
                    interpreter.runStep();
                }
            }
            stepSimul = false;
        }

        std::vector<TrivialShaping::Rectangle> rectangles;
        std::vector<TrivialShaping::Rectangle> inserted;
        inserted = renderGrid(interpreter.mGrid);
        rectangles.insert(rectangles.end(), inserted.begin(), inserted.end());

        /*if (interpreter.mRoot->nodes.at(interpreter.mCurrentBranch->currentStep)->isRuleNode())
        {
            RuleNode * node = dynamic_cast<RuleNode*>(interpreter.mCurrentBranch->nodes.at(interpreter.mCurrentBranch->currentStep).get());

            inserted = renderPotential(node->mPotentials, interpreter.mGrid);
            rectangles.insert(rectangles.end(), inserted.begin(), inserted.end());
        }*/


        shapes.updateInstances(rectangles);
        shapes.render();
    }

    //std::cout << interpreter.mGrid;
    std::exit(EXIT_SUCCESS);
}
