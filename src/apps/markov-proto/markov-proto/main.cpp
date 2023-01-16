#include "markovjunior/Grid.h"
#include "markovjunior/ImageHelpers.h"
#include "markovjunior/RuleNode.h"

#include <chrono>
#include <ctime>
#include <graphics/ApplicationGlfw.h>
#include <graphics/CameraUtilities.h>
#include <graphics/TrivialShaping.h>
#include <imgui.h>
#include <imguiui/ImguiUi.h>
#include <markovjunior/Interpreter.h>
#include <math/Color.h>
#include <math/Matrix.h>
#include <math/Rectangle.h>
#include <memory>
#include <platform/Filesystem.h>
#include <ratio>
#include <renderer/commons.h>
#include <string>
#include <thread>

//SHITTY SEED : -1703636407 size 37

using namespace ad::markovjunior;
using namespace ad::graphics;

constexpr ad::graphics::Size2<int> gWindowSize{1400, 800};
constexpr float gMarkovDrawSize = 800.f;
inline std::mt19937
    randomSeedgenerator((std::uint32_t)std::chrono::system_clock::now().time_since_epoch().count());
inline int gSeed = randomSeedgenerator();
inline int gSize = 29;
inline int gSteps = 1;
inline ad::filesystem::path filename{"snaclvl.xml"};

inline ApplicationGlfw application{"Markovjunior", gWindowSize,
                                   ApplicationFlag::Window_Keep_Ratio};
inline bool runSimulation = false;
inline bool stepSimul = false;

std::vector<TrivialShaping::Rectangle> renderGrid(const Grid & aGrid)
{
    std::vector<TrivialShaping::Rectangle> result;

    float cellSize = gMarkovDrawSize / (float)aGrid.mSize.width();

    for (int z = 0; z < aGrid.mSize.depth(); z++) {
        for (int y = 0; y < aGrid.mSize.height(); y++) {
            for (int x = 0; x < aGrid.mSize.width(); x++) {
                unsigned char value = aGrid.mCharacters.at(
                    aGrid.mState.at(aGrid.getFlatGridIndex({x, y, z})));
                std::string color = std::string(1, static_cast<char>(value));
                result.push_back({{{x * cellSize, y * cellSize}, {cellSize, cellSize}},
                                  gColorMatching.at(color)});
            }
        }
    }

    return result;
}

std::vector<TrivialShaping::Rectangle>
renderPotential(const std::vector<std::vector<int>> & aPotentials, const Grid & aGrid)
{
    std::vector<TrivialShaping::Rectangle> result;

    float cellSize = gMarkovDrawSize / (float)aGrid.mSize.width();

    for (unsigned int i = 0; i != aPotentials.size(); i++) {
        for (int z = 0; z < aGrid.mSize.depth(); z++) {
            for (int y = 0; y < aGrid.mSize.height(); y++) {
                for (int x = 0; x < aGrid.mSize.width(); x++) {
                    if (!aPotentials.at(i).empty()) {
                        unsigned char value = aGrid.mCharacters.at(i);
                        std::string color = std::string(1, static_cast<char>(value));
                        ad::math::sdr::Rgb colorRgb = gColorMatching.at(color);
                        int flatIndex = aGrid.getFlatGridIndex({x, y, z});
                        int potential = aPotentials.at(i).at(flatIndex);
                        if (potential > 0) {
                            colorRgb.r() /= potential;
                            colorRgb.g() /= potential;
                            colorRgb.b() /= potential;
                            result.push_back(
                                {{{x * cellSize, y * cellSize}, {cellSize, cellSize}},
                                 colorRgb});
                        }
                    }
                }
            }
        }
    }

    return result;
}

inline ad::math::Rectangle<GLfloat>
getViewedRectangle(ad::math::Position<2, float> aCameraPosition, float aViewportRatio)
{
    return ad::math::Rectangle<GLfloat>{
        aCameraPosition,
        ad::math::makeSizeFromHeight((float)gWindowSize.height(), aViewportRatio)};
}

inline void callbackKeyboard(int key, int , int action, int)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        application.getAppInterface()->requestCloseApplication();
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        runSimulation = !runSimulation;
    }
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        stepSimul = true;
    }
}

int main()
{
    std::shared_ptr<Interpreter> interpreter =
        std::make_shared<Interpreter>("/home/franz/gamedev/snac-assets/markov", filename,
                                      ad::math::Size<3, int>{gSize, gSize, 1}, gSeed);

    interpreter->setup();

    application.getAppInterface()->registerKeyCallback(&callbackKeyboard);

    ad::imguiui::ImguiUi debugUi{application};

    TrivialShaping shapes{application.getAppInterface()->getWindowSize()};

    ad::math::Rectangle<GLfloat> viewed = getViewedRectangle(
        {0.f, 0.f},
        ad::math::getRatio<GLfloat>(application.getAppInterface()->getWindowSize()));

    setViewedRectangle(shapes, viewed);

    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> endTime;

    startTime = std::chrono::high_resolution_clock::now();

    while (application.nextFrame()) {
        application.getAppInterface()->clear();

        debugUi.newFrame();
        std::chrono::time_point<std::chrono::high_resolution_clock> startStep;
        std::chrono::time_point<std::chrono::high_resolution_clock> endStep;

        if (runSimulation || stepSimul) {
            for (int i = 0; i != gSteps; i++) {
                if (interpreter->mCurrentBranch != nullptr) {
                    startStep = std::chrono::high_resolution_clock::now();
                    interpreter->runStep();
                    endStep = std::chrono::high_resolution_clock::now();
                    endTime = std::chrono::high_resolution_clock::now();
                }
            }
            stepSimul = false;
        }

        ImGui::SetNextWindowPos(ImVec2(gMarkovDrawSize + 10, 10), ImGuiCond_Always);
        ImGui::SetNextWindowSize(
            ImVec2((float)(gWindowSize.width() - gMarkovDrawSize - 20),
                   (float)(gWindowSize.height() - 20)),
            ImGuiCond_Once);
        ImGui::Begin("haha");
        if (ImGui::Button("Play")) {
            runSimulation = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop")) {
            runSimulation = false;
        }
        if (!runSimulation) {
            ImGui::SameLine();
            if (ImGui::Button("Step")) {
                stepSimul = true;
            }
        }
        ImGui::InputInt("seed", &gSeed);
        ImGui::InputInt("size", &gSize);
        ImGui::SliderInt("steps", &gSteps, 1, 1000);
        ImGui::Text("Frame duration");
        std::chrono::duration<double, std::milli> time{endStep - startStep};
        std::ostringstream oStep;
        oStep << time.count();
        ImGui::Text("%s", oStep.str().c_str());
        ImGui::Text("Simul duration");
        std::chrono::duration<double, std::milli> simulTime{endTime - startTime};
        std::ostringstream oSimul;
        oSimul << simulTime.count();
        ImGui::Text("%s", oSimul.str().c_str());
        if (ImGui::Button("Restart")) {
            startTime = std::chrono::high_resolution_clock::now();
            interpreter = std::make_shared<Interpreter>("/home/franz/snac-assets/markov", filename,
                ad::math::Size<3, int>{gSize, gSize, 1}, gSeed);
            interpreter->setup();
        }
        interpreter->mRoot->debugRender();
        ImGui::End();
        ImGui::ShowDemoWindow();

        std::vector<TrivialShaping::Rectangle> rectangles;
        std::vector<TrivialShaping::Rectangle> inserted;
        inserted = renderGrid(interpreter->mGrid);
        rectangles.insert(rectangles.end(), inserted.begin(), inserted.end());

        /*if
        (interpreter.mRoot->nodes.at(interpreter.mCurrentBranch->currentStep)->isRuleNode())
        {
            RuleNode * node =
        dynamic_cast<RuleNode*>(interpreter.mCurrentBranch->nodes.at(interpreter.mCurrentBranch->currentStep).get());

            inserted = renderPotential(node->mPotentials, interpreter.mGrid);
            rectangles.insert(rectangles.end(), inserted.begin(), inserted.end());
        }*/

        shapes.updateInstances(rectangles);
        shapes.render();
        debugUi.render();
        debugUi.renderBackend();
    }

    // std::cout << interpreter.mGrid;
    std::exit(EXIT_SUCCESS);
}
