#include "Path.h"
#include <limits>
#include <queue>

namespace ad {
namespace markovjunior {

void addToCandidates(
        math::Position<3, int> aPos,
        const math::Vec<3, int> & aDir,
        const std::vector<int> & aGenerations,
        std::vector<math::Vec<3, int>> & aCandidates,
        const Grid & aGrid,
        int aGenerationValue
        )
{
    if (aGenerations.at(aGrid.getFlatGridIndex(aPos + aDir)) == aGenerationValue)
    {
        aCandidates.emplace_back(aDir);
    }
}

void Path::pushToFrontier(
        int aGenerationIndex,
        const math::Position<3, int> & aPos,
        std::vector<int> & aGenerations,
        std::queue<std::tuple<int, math::Position<3, int>>> & aFrontier)
{
    int flatIndex = mGrid->getFlatGridIndex(aPos);
    unsigned char value = mGrid->mState.at(flatIndex);

    if (aGenerations.at(flatIndex) == -1 &&
            ((mSubstrate & 1 << value) != 0 ||
             (mStart & 1 << value) != 0))
    {
        if ((mSubstrate & 1 << value) != 0)
        {
            aFrontier.emplace(aGenerationIndex, aPos);
        }

        aGenerations.at(flatIndex) = aGenerationIndex;
    }
}

bool Path::run()
{
    std::queue<std::tuple<int, math::Position<3, int>>> frontier;
    std::vector<math::Position<3, int>> startPosition;
    Grid & grid = *mGrid;
    std::vector<int> generations(grid.mState.size(), -1);

    for (int z = 0; z < grid.mSize.depth(); z++)
    {
        for (int y = 0; y < grid.mSize.height(); y++)
        {
            for (int x = 0; x < grid.mSize.width(); x++)
            {
                int flatIndex = grid.getFlatGridIndex({x, y, z});

                unsigned char stateInPos = grid.mState.at(flatIndex);

                if ((mStart & 1 << stateInPos) != 0)
                {
                    startPosition.emplace_back(x, y, z);
                }
                if ((mFinish & 1 << stateInPos) != 0)
                {
                    generations.at(flatIndex) = 0;
                    frontier.emplace(0, math::Position<3, int>{x, y, z});
                }
            }
        }
    }

    if (startPosition.empty() || frontier.empty())
    {
        return false;
    }

    while(!frontier.empty())
    {
        auto [genValue, pos] = frontier.front();
        frontier.pop();

        for (auto dir : getAllPossibleDirections(
                    pos,
                    grid.mSize,
                    mEdges,
                    mVertices))
        {
            pushToFrontier(genValue + 1, pos + dir, generations, frontier);
        }
    }

    if (std::find_if(startPosition.begin(),
                startPosition.end(),
                [&](const math::Position<3, int> startPos) {
                    return generations.at(grid.getFlatGridIndex(startPos)) > 0;
                }) == startPosition.end())
    {
        return false;
    }

    std::mt19937 localRandom(mInterpreter->mRandom);

    //TODO test with numeric limits
    double min = grid.mSize.width() * grid.mSize.height() * grid.mSize.depth();
    double max = -2.f;
    math::Position<3, int> argmin = math::Position<3, int>::Zero();
    math::Position<3, int> argmax = math::Position<3, int>::Zero();
    
    for (auto pos : startPosition)
    {
        int genValue = generations.at(grid.getFlatGridIndex(pos));

        if (genValue == -1)
        {
            continue;
        }

        double dg = genValue;
        double noise = 0.1 * mInterpreter->mProbabilityDistribution(localRandom);

        if (dg + noise < min)
        {
            min = dg + noise;
            argmin = pos;
        }

        if (dg + noise > max)
        {
            max = dg + noise;
            argmax = pos;
        }
    }

    math::Position<3, int> pen = mLongest ? argmax : argmin;
    math::Vec<3, int> chosenDir = getDirection(pen, {0, 0, 0}, generations, localRandom);

    pen += chosenDir;

    while(generations.at(grid.getFlatGridIndex(pen)) != 0)
    {
        grid.mState.at(grid.getFlatGridIndex(pen)) = mValue;
        mInterpreter->mChanges.push_back(pen);
        chosenDir = getDirection(pen, chosenDir, generations, localRandom);
        pen += chosenDir;
    }

    return true;
}

math::Vec<3, int> Path::getDirection(const math::Position<3, int> &aPos, const math::Vec<3, int> &aDirection, const std::vector<int> &aGenerations, std::mt19937 &aRandom)
{
    std::vector<math::Vec<3, int>> candidateDirection;
    const Grid & grid = *mGrid;
    int flatIndex = grid.getFlatGridIndex(aPos);
    int g = aGenerations.at(flatIndex) - 1;

    if (!mVertices && !mEdges)
    {
        if (aDirection.x() != 0 || aDirection.y() != 0 || aDirection.z() != 0)
        {
            math::Position<3, int> uniqueCandidate = aPos + aDirection;

            if (
                mInertia &&
                uniqueCandidate.x() >= 0 &&
                uniqueCandidate.y() >= 0 &&
                uniqueCandidate.z() >= 0 &&
                uniqueCandidate.x() < grid.mSize.width() &&
                uniqueCandidate.y() < grid.mSize.height() &&
                uniqueCandidate.z() < grid.mSize.depth() &&
                aGenerations.at(grid.getFlatGridIndex(uniqueCandidate)) == g)
            {
                return aDirection;
            }
        }

        if (aPos.x() > 0)
        {
            addToCandidates(
                    aPos,
                    {-1, 0, 0},
                    aGenerations,
                    candidateDirection,
                    grid,
                    g);
        }
        if (aPos.x() < grid.mSize.width() - 1)
        {
            addToCandidates(
                    aPos,
                    {1, 0, 0},
                    aGenerations,
                    candidateDirection,
                    grid,
                    g);
        }
        if (aPos.y() > 0)
        {
            addToCandidates(
                    aPos,
                    {0, -1, 0},
                    aGenerations,
                    candidateDirection,
                    grid,
                    g);
        }
        if (aPos.y() < grid.mSize.height() - 1)
        {
            addToCandidates(
                    aPos,
                    {0, 1, 0},
                    aGenerations,
                    candidateDirection,
                    grid,
                    g);
        }
        if (aPos.z() > 0)
        {
            addToCandidates(
                    aPos,
                    {0, 0, -1},
                    aGenerations,
                    candidateDirection,
                    grid,
                    g);
        }
        if (aPos.z() < grid.mSize.depth() - 1)
        {
            addToCandidates(
                    aPos,
                    {0, 0, 1},
                    aGenerations,
                    candidateDirection,
                    grid,
                    g);
        }

        int randomIndex = aRandom() % candidateDirection.size();
        return candidateDirection.at(randomIndex);
    }
    else
    {
        auto allPossibleDirections = getAllPossibleDirections(
                aPos,
                grid.mSize,
                mEdges,
                mVertices
                );

        for (auto dir : allPossibleDirections)
        {
            addToCandidates(aPos, dir, aGenerations, candidateDirection, grid, g);
        }

        if (mInertia && (aDirection.x() != 0 || aDirection.y() != 0 || aDirection.z() != 0))
        {
            double maxScalar = -4;
            math::Vec<3, int> result = math::Vec<3, int>::Zero();

            for (auto dir : candidateDirection)
            {
                double noise = 0.1 * mInterpreter->mProbabilityDistribution(aRandom);

                double cos = dir.dot(aDirection) / std::sqrt(dir.getNormSquared() * aDirection.getNormSquared());

                if (cos + noise > maxScalar)
                {
                    maxScalar = cos + noise;
                    result = dir;
                }
            }

            return result;
        }
        else
        {
            int randomIndex = aRandom() % candidateDirection.size();
            return candidateDirection.at(randomIndex);
        }
    }
}

std::vector<math::Vec<3, int>> Path::getAllPossibleDirections(const math::Position<3, int> &aPos, const math::Size<3, int> &aSize, bool aEdges, bool aVertices)
{
    std::vector<math::Vec<3, int>> result;
    if (aPos.x() > 0)
    {
        result.emplace_back(-1, 0, 0);
    }
    if (aPos.x() < aSize.width() - 1)
    {
        result.emplace_back(1, 0, 0);
    }
    if (aPos.y() > 0)
    {
        result.emplace_back(0, -1, 0);
    }
    if (aPos.y() < aSize.height() - 1)
    {
        result.emplace_back(0, 1, 0);
    }
    if (aPos.z() > 0)
    {
        result.emplace_back(0, 0, -1);
    }
    if (aPos.z() < aSize.depth() - 1)
    {
        result.emplace_back(0, 0, 1);
    }

    if (aEdges)
    {
        if (aPos.x() > 0 && aPos.y() > 0)
        {
            result.emplace_back(-1, -1, 0);
        }
        if (aPos.x() > 0 && aPos.y() < aSize.height() - 1)
        {
            result.emplace_back(-1, 1, 0);
        }
        if (aPos.x() < aSize.width() - 1 && aPos.y() > 0)
        {
            result.emplace_back(1, -1, 0);
        }
        if (aPos.x() < aSize.width() - 1 && aPos.y() < aSize.height() - 1)
        {
            result.emplace_back(1, 1, 0);
        }

        if (aPos.x() > 0 && aPos.z() > 0)
        {
            result.emplace_back(-1, 0, -1);
        }
        if (aPos.x() > 0 && aPos.z() < aSize.depth() - 1)
        {
            result.emplace_back(-1, 0, 1);
        }
        if (aPos.x() < aSize.width() - 1 && aPos.z() > 0)
        {
            result.emplace_back(1, 0, -1);
        }
        if (aPos.x() < aSize.width() - 1 && aPos.z() < aSize.depth() - 1)
        {
            result.emplace_back(1, 0, 1);
        }

        if (aPos.y() > 0 && aPos.z() > 0)
        {
            result.emplace_back(0, -1, -1);
        }
        if (aPos.y() > 0 && aPos.z() < aSize.depth() - 1)
        {
            result.emplace_back(0, -1, 1);
        }
        if (aPos.y() < aSize.height() - 1 && aPos.z() > 0)
        {
            result.emplace_back(0, 1, -1);
        }
        if (aPos.y() < aSize.height() - 1 && aPos.z() < aSize.depth() - 1)
        {
            result.emplace_back(0, 1, 1);
        }
    }

    if (aVertices)
    {
        if (aPos.x() > 0 && aPos.y() > 0 && aPos.z() > 0)
        {
            result.emplace_back(-1, -1, -1);
        }
        if (aPos.x() > 0 && aPos.y() > 0 && aPos.z() < aSize.depth() - 1)
        {
            result.emplace_back(-1, -1, 1);
        }
        if (aPos.x() > 0 && aPos.y() < aSize.height() - 1 && aPos.z() > 0)
        {
            result.emplace_back(-1, 1, -1);
        }
        if (aPos.x() > 0 && aPos.y() < aSize.height() - 1 && aPos.z() < aSize.depth() - 1)
        {
            result.emplace_back(-1, 1, 1);
        }

        if (aPos.x() < aSize.width() - 1 && aPos.y() > 0 && aPos.z() > 0)
        {
            result.emplace_back(1, -1, -1);
        }
        if (aPos.x() < aSize.width() - 1 && aPos.y() > 0 && aPos.z() < aSize.depth() - 1)
        {
            result.emplace_back(1, -1, 1);
        }
        if (aPos.x() < aSize.width() - 1 && aPos.y() < aSize.height() - 1 && aPos.z() > 0)
        {
            result.emplace_back(1, 1, -1);
        }
        if (aPos.x() < aSize.width() - 1 && aPos.y() < aSize.height() - 1 && aPos.z() < aSize.depth() - 1)
        {
            result.emplace_back(1, 1, 1);
        }
    }

    return result;
}

}
}
