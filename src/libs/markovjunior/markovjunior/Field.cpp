#include "Field.h"
#include "Grid.h"
#include "Rule.h"

#include <math/Vector.h>

#include <tuple>
#include <queue>

namespace ad {
namespace markovjunior {

bool Field::compute(std::vector<int> & aPotential, const Grid & aGrid)
{
    math::Size<3, int> gridSize = aGrid.mSize;
    std::queue<std::tuple<int, math::Position<3, int>>> potentialPosQueue;

    for (int z = 0; z < gridSize.depth(); z++)
    {
        for (int y = 0; y < gridSize.height(); y++)
        {
            for (int x = 0; x < gridSize.width(); x++)
            {
                int flatIndex = aGrid.getFlatGridIndex({x, y, z});

                aPotential.at(flatIndex) = -1;

                unsigned char value = aGrid.mState.at(flatIndex);
                
                if ((mZero & 1 << value) != 0)
                {
                    aPotential.at(flatIndex) = 0;
                    potentialPosQueue.emplace(0, math::Position<3, int>{x, y, z});
                }
            }
        }
    }

    if (potentialPosQueue.empty())
    {
        return false;
    }

    while(!potentialPosQueue.empty())
    {
        auto [potential, potentialPos] = potentialPosQueue.front();
        potentialPosQueue.pop();

        auto neighbors = getNeighbors(potentialPos, gridSize);

        for (auto neighbor : neighbors)
        {
            int flatIndex = aGrid.getFlatGridIndex(neighbor);
            unsigned char value = aGrid.mState.at(flatIndex);

            if (aPotential.at(flatIndex) == -1 && (mSubstrate & 1 << value) != 0)
            {
                potentialPosQueue.emplace(potential + 1, neighbor);
                aPotential.at(flatIndex) = potential + 1;
            }
        }

    }

    return true;
}

std::optional<int> Field::deltaPointwise(
        const std::vector<unsigned char> & aState,
        const Rule & aRule,
        const math::Position<3, int> & aPos,
        const std::vector<Field> & aFields,
        const std::vector<std::vector<int>> & aPotentials,
        const math::Size<3, int> & aGridSize)
{
    int sum = 0;
    for (int z = 0; z < aRule.mInputSize.depth(); z++)
    {
        for (int y = 0; y < aRule.mInputSize.height(); y++)
        {
            for (int x = 0; x < aRule.mInputSize.width(); x++)
            {
                unsigned char newValue = aRule.mOutputs.at(getFlatIndex(
                            {x, y, z},
                            aRule.mOutputSize
                            ));

                if (newValue != 0xff && (aRule.mInputs.at(getFlatIndex(
                                    {x, y, z},
                                    aRule.mInputSize
                                    ))  & 1 << newValue) == 0)
                {
                    int flatIndex = getFlatIndex(
                            aPos + math::Vec<3, int>{x, y, z},
                            aGridSize
                            );

                    int newPotential = aPotentials.at(newValue).at(flatIndex);
                    
                    if (newPotential == -1)
                    {
                        return std::nullopt;
                    }

                    unsigned char oldValue = aState.at(flatIndex);
                    int oldPotential = aPotentials.at(oldValue).at(flatIndex);

                    sum += newPotential - oldPotential;

                    if (!aFields.empty())
                    {
                        Field oldField = aFields.at(oldValue);

                        if (oldField.mSubstrate != -1 && oldField.mInversed)
                        {
                            sum += 2 * oldPotential;
                        }
                        Field newField = aFields.at(newValue);
                        if (newField.mSubstrate != -1 && newField.mInversed)
                        {
                            sum -= 2 * newPotential;
                        }
                    }
                }
            }
        }
    }

    return sum;
}

}
}
