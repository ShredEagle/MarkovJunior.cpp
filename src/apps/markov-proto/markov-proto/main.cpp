#include "markovjunior/Grid.h"
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

constexpr float gViewedHeight = 500.;

const std::map<std::string, ad::math::sdr::Rgb> colorMatching = {
    {"B", ad::math::sdr::gBlack},
    {"W", ad::math::sdr::gWhite},
    {"K", ad::math::sdr::Rgb{255, 112, 146}},
    {"O", ad::math::sdr::Rgb{255, 153, 0}},
    {"R", ad::math::sdr::gRed},
    {"C", ad::math::sdr::gCyan},
    {"G", ad::math::sdr::gGreen},
    {"U", ad::math::sdr::gBlue},
    {"M", ad::math::sdr::gMagenta},
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

inline ad::math::Rectangle<GLfloat> getViewedRectangle(ad::math::Position<2, float> aCameraPosition, float aViewportRatio)
{
    return ad::math::Rectangle<GLfloat>{
        aCameraPosition,
        ad::math::makeSizeFromHeight(gViewedHeight, aViewportRatio)
    }.centered();
}

int main()
{
    Interpreter interpreter("/home/franz/gamedev/MarkovJunior.cpp/assets/backtracer.xml", {31, 31, 1}, 345);
    //std::cout << interpreter.mGrid << std::endl;
    interpreter.setup();
    //std::cout << interpreter.mGrid << std::endl;
    constexpr ad::graphics::Size2<int> gWindowSize{500, 500};
    ad::graphics::ApplicationGlfw application(
            "MarkovJunior viewer", gWindowSize, ApplicationFlag::Window_Keep_Ratio);

    TrivialShaping shapes{application.getAppInterface()->getWindowSize()};

    ad::math::Rectangle<GLfloat> viewed = getViewedRectangle(
            {gViewedHeight / 2., gViewedHeight / 2.},
            ad::math::getRatio<GLfloat>(
                application.getAppInterface()->getWindowSize()));

    setViewedRectangle(shapes, viewed);

    while(application.nextFrame())
    {
        application.getAppInterface()->clear();

        if (interpreter.mCurrentBranch != nullptr)
        {
            interpreter.runStep();
        }

        shapes.updateInstances(renderGrid(interpreter.mGrid));

        shapes.render();
    }

    //std::cout << interpreter.mGrid;
    std::exit(EXIT_SUCCESS);
}
