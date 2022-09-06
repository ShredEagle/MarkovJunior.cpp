#include <markovjunior/Interpreter.h>

using namespace ad::markovjunior;
int main()
{
    Interpreter interpreter("/home/franz/gamedev/MarkovJunior.cpp/assets/backtracer.xml", {30, 30, 1}, 2);
    std::cout << interpreter.mGrid << std::endl;
    interpreter.setup();
    std::cout << interpreter.mGrid << std::endl;
    SequenceNode & root = *interpreter.mRoot;
    std::cout << root;
    while(interpreter.mCurrentBranch != nullptr)
    {
        interpreter.runStep();
    std::cout << interpreter.mGrid;
    }
    return 0;
}
