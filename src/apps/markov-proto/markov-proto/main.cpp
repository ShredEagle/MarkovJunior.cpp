#include "markovjunior/Grid.h"
#include "markovjunior/RuleNode.h"
#include <graphics/CameraUtilities.h>
#include <imgui.h>
#include <imguiui/ImguiUi.h>
#include <markovjunior/Interpreter.h>
#include <graphics/ApplicationGlfw.h>
#include <graphics/TrivialShaping.h>

#include <chrono>
#include <math/Color.h>
#include <math/Matrix.h>
#include <math/Rectangle.h>
#include <memory>
#include <ratio>
#include <renderer/commons.h>
#include <string>
#include <thread>

using namespace ad::markovjunior;
using namespace ad::graphics;

constexpr ad::graphics::Size2<int> gWindowSize{1400, 800};
constexpr float gMarkovDrawSize = 800.f;
inline int gSeed = 0;
inline int gSize = 59;
inline int gSteps = 1;
inline std::string filename = "/home/franz/gamedev/MarkovJunior.cpp/assets/bernouilli_percolation.xml";

inline ApplicationGlfw application{"Markovjunior", gWindowSize, ApplicationFlag::Window_Keep_Ratio};
inline bool runSimulation = false;
inline bool stepSimul = false;

const std::map<std::string, ad::math::sdr::Rgb> colorMatching = {
    {"B", ad::math::sdr::gBlack},
    {"W", ad::math::sdr::Rgb{0xff, 0xf1, 0xe8}},
    {"t", ad::math::sdr::Rgb{0x32, 0x3c, 39}},
    {"K", ad::math::sdr::Rgb{255, 112, 146}},
    {"O", ad::math::sdr::Rgb{255, 153, 0}},
    {"E", ad::math::sdr::Rgb{0, 0x87, 0x51}},
    {"D", ad::math::sdr::Rgb{0x5f, 0x57, 0x4f}},
    {"N", ad::math::sdr::Rgb{0xab, 0x52, 0x36}},
    {"R", ad::math::sdr::gRed},
    {"C", ad::math::sdr::gCyan},
    {"G", ad::math::sdr::Rgb{0x0, 0xe4, 0x36}},
    {"U", ad::math::sdr::Rgb{0x29, 0xad, 0xff}},
    {"P", ad::math::sdr::gMagenta},
    {"Y", ad::math::sdr::Rgb{0xff, 0xec, 0x27}},
    {"A", ad::math::sdr::Rgb{0xc2, 0xc3, 0xc7}}
};

std::vector<TrivialShaping::Rectangle> renderGrid(const Grid & aGrid)
{
    std::vector<TrivialShaping::Rectangle> result;

    float cellSize = gMarkovDrawSize / (float)aGrid.mSize.width();

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

    float cellSize = gMarkovDrawSize / (float)aGrid.mSize.width();

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
        ad::math::makeSizeFromHeight((float)gWindowSize.height(), aViewportRatio)
    };
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
    std::shared_ptr<Interpreter> interpreter = std::make_shared<Interpreter>(
            filename,
            ad::math::Size<3, int>{59, 59, 1},
            gSeed);

    interpreter->setup();

    application.getAppInterface()->registerKeyCallback(&callbackKeyboard);

    ad::imguiui::ImguiUi debugUi{application};

    TrivialShaping shapes{application.getAppInterface()->getWindowSize()};

    ad::math::Rectangle<GLfloat> viewed = getViewedRectangle(
            {0., 0.},
            ad::math::getRatio<GLfloat>(
                application.getAppInterface()->getWindowSize()));

    setViewedRectangle(shapes, viewed);

    while(application.nextFrame())
    {
        application.getAppInterface()->clear();
        
        debugUi.newFrame();
        std::chrono::time_point<std::chrono::high_resolution_clock> startStep;
        std::chrono::time_point<std::chrono::high_resolution_clock> endStep;

        if (runSimulation || stepSimul)
        {
            for (int i = 0; i != gSteps; i++)
            {
                if (interpreter->mCurrentBranch != nullptr)
                {
                    startStep = std::chrono::high_resolution_clock::now();
                    interpreter->runStep();
                    endStep = std::chrono::high_resolution_clock::now();
                }
            }
            stepSimul = false;
        }

        ImGui::SetNextWindowPos(ImVec2(gMarkovDrawSize + 10, 10), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(gWindowSize.width() - gMarkovDrawSize - 20, gWindowSize.height() - 20), ImGuiCond_Once);
        ImGui::Begin("haha");
        if (ImGui::Button("Play"))
        {
            runSimulation = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop"))
        {
            runSimulation = false;
        }
        if (!runSimulation)
        {
            ImGui::SameLine();
            if (ImGui::Button("Step"))
            {
                stepSimul = true;
            }
        }
        ImGui::InputInt("seed", &gSeed);
        ImGui::InputInt("size", &gSize);
        ImGui::SliderInt("steps", &gSteps, 1, 100);
        ImGui::Text("Frame duration");
        std::chrono::duration<double, std::milli> time{endStep - startStep};
        std::ostringstream o;
        o << time;
        ImGui::Text(o.str().c_str());
        if(ImGui::Button("Restart"))
        {
            interpreter = std::make_shared<Interpreter>(
                filename,
                ad::math::Size<3, int>{gSize, gSize, 1},
                gSeed);
            interpreter->setup();
        }
        ImGui::End();
        ImGui::ShowDemoWindow();

        std::vector<TrivialShaping::Rectangle> rectangles;
        std::vector<TrivialShaping::Rectangle> inserted;
        inserted = renderGrid(interpreter->mGrid);
        rectangles.insert(rectangles.end(), inserted.begin(), inserted.end());

        /*if (interpreter.mRoot->nodes.at(interpreter.mCurrentBranch->currentStep)->isRuleNode())
        {
            RuleNode * node = dynamic_cast<RuleNode*>(interpreter.mCurrentBranch->nodes.at(interpreter.mCurrentBranch->currentStep).get());

            inserted = renderPotential(node->mPotentials, interpreter.mGrid);
            rectangles.insert(rectangles.end(), inserted.begin(), inserted.end());
        }*/


        shapes.updateInstances(rectangles);
        shapes.render();
        debugUi.render();
    }

    //std::cout << interpreter.mGrid;
    std::exit(EXIT_SUCCESS);
}
