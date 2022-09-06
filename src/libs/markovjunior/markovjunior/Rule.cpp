#include "Rule.h"

#include "Constants.h"
#include <iostream>

namespace ad {
namespace markovjunior {

//From https://web.archive.org/web/20190108223926/http://www.hackersdelight.org/hdcodetxt/ntz.c.txt
int countTrailingZeroes(int x)
{

    if (x == 0) return(32);
    int n = 1;
    if ((x & 0x0000FFFF) == 0) {n = n + 16; x = x >> 16;}
    if ((x & 0x000000FF) == 0) {n = n + 8; x = x >> 8;}
    if ((x & 0x0000000F) == 0) {n = n + 4; x = x >> 4;}
    if ((x & 0x00000003) == 0) {n = n + 2; x = x >> 2;}
    return n - (x & 1);
}

void Rule::setupShifts(int aCount)
{
    mInputShifts.insert(mInputShifts.end(), aCount, std::vector<math::Position<3, int>>{});

    for (int z = 0; z < mInputSize.depth(); z++)
    {
        for (int y = 0; y < mInputSize.height(); y++)
        {
            for (int x = 0; x < mInputSize.width(); x++)
            {
                int w = mInputs.at(getFlatIndex({x, y, z}, mInputSize));

                for (int c = 0; c < aCount; c++, w >>= 1)
                {
                    if (w & 1)
                    {
                        mInputShifts.at(c).emplace_back(x, y, z);
                    }
                }
            }
        }
    }
    

    if (mInputSize == mOutputSize)
    {
        mOutputShifts = mInputShifts;

        for (auto shifts :mOutputShifts)
        {
            shifts.clear();
        }

        for (int z = 0; z < mOutputSize.depth(); z++)
        {
            for (int y = 0; y < mOutputSize.height(); y++)
            {
                for (int x = 0; x < mOutputSize.width(); x++)
                {
                    int o = mOutputs.at(getFlatIndex({x, y, z}, mOutputSize));

                    if (o != gWildcardShiftValue)
                    {
                        mOutputShifts.at(o).emplace_back(x, y, z);
                    }
                    else
                    {
                        for (int c = 0; c < aCount; c++)
                        {
                            mOutputShifts.at(c).emplace_back(x, y, z);
                        }
                    }
                }
            }
        }
    }

    int wildcard = (1 << aCount) - 1;

    for (int i = 0; i < mInputs.size(); i++)
    {
        mByteInput.push_back(
                mInputs.at(i) == wildcard
                ? gWildcardShiftValue : static_cast<char>(countTrailingZeroes(mInputs.at(i))));
    }
}

Rule::Rule(
        std::vector<int> aInput,
        math::Size<3, int> aInputSize,
        std::vector<char> aOutput,
        math::Size<3, int> aOutputSize, int aCount, double aP) :
    mInputs{aInput},
    mInputSize{aInputSize},
    mOutputs{aOutput},
    mOutputSize{aOutputSize},
    mP{aP}
{
    setupShifts(aCount);
}

Rule::Rule(const pugi::xml_node & aXmlNode, const Grid & aGridIn, const Grid & aGridOut) :
    mInputSize{-1, -1, -1},
    mOutputSize{-1, -1, -1}
{
    std::string inString = aXmlNode.attribute("in").as_string("");
    std::string outString = aXmlNode.attribute("out").as_string("");


    if (inString.empty())
    {
        std::cout << "NO INSTRING" << std::endl;
        //TODO put spdlog
    }

    if (outString.empty())
    {
        std::cout << "NO OUTSTRING" << std::endl;
        //TODO put spdlog
    }

    auto [inRect, inputSize] = parsePatternString(inString);
    auto [outRect, outputSize] = parsePatternString(outString);
    mInputSize = inputSize;
    mOutputSize = outputSize;

    mInputs.reserve(inRect.size());
    mOutputs.reserve(outRect.size());

    for (auto candidateInput : inRect)
    {
        mInputs.push_back(aGridIn.mWaves.at(candidateInput));
    }

    for (auto candidateOutput : outRect)
    {
        if (candidateOutput == "*"[0])
        {
            mOutputs.push_back(gWildcardShiftValue);
        }
        else
        {
            mOutputs.push_back(aGridOut.mValues.at(candidateOutput));
        }
    }

    mP = aXmlNode.attribute("p").as_double(1.0);

    setupShifts(aGridIn.mCharacters.size());
}

std::tuple<std::vector<char>, math::Size<3, int>> Rule::parsePatternString(
        const std::string & aPatternString)
{
    std::vector<std::vector<std::string>> lines = splitPatternString(aPatternString);
    math::Size<3, int> patternSize{
        lines.at(0).at(0).length(),
        lines.at(0).size(),
        lines.size()
    };
    std::vector<char> result;

    for (int z = 0; z < patternSize.depth(); z++)
    {
        std::vector<std::string> plane = lines.at(patternSize.depth() - 1 - z);
        if (plane.size() != patternSize.height())
        {
            std::cout << "PATTERN is not rectangular" << std::endl;
            return {{}, {-1, -1, -1}};
        }

        for (int y = 0; y < patternSize.height(); y++)
        {
            std::string line = plane.at(y);
            if (line.size() != patternSize.width())
            {
                std::cout << "PATTERN is not rectangular" << std::endl;
                return {{}, {-1, -1, -1}};
            }

            for (int x = 0; x < patternSize.width(); x++)
            {
                result.push_back(line.at(x));
            }
        }
    }

    return {result, patternSize};
}

Rule Rule::reflect()
{
    std::vector<int> newInputs = mInputs;
    for (int z = 0; z < mInputSize.depth(); z++)
    {
        for (int y = 0; y < mInputSize.height(); y++)
        {
            for (int x = 0; x < mInputSize.width(); x++)
            {
                newInputs.at(getFlatIndex({x, y, z}, mInputSize)) = 
                    mInputs.at(getFlatIndex({mInputSize.width() - 1 - x, y, z}, mInputSize));
            }
        }
    }

    std::vector<char> newOutputs = mOutputs;
    for (int z = 0; z < mOutputSize.depth(); z++)
    {
        for (int y = 0; y < mOutputSize.height(); y++)
        {
            for (int x = 0; x < mOutputSize.width(); x++)
            {
                newOutputs.at(getFlatIndex({x, y, z}, mOutputSize)) = 
                    mOutputs.at(getFlatIndex({mOutputSize.width() - 1 - x, y, z}, mOutputSize));
            }
        }
    }

    return Rule(newInputs, mInputSize, newOutputs, mOutputSize, mInputShifts.size(), mP);
}

Rule Rule::rotate()
{
    std::vector<int> newInputs = mInputs;
    for (int z = 0; z < mInputSize.depth(); z++)
    {
        for (int y = 0; y < mInputSize.width(); y++)
        {
            for (int x = 0; x < mInputSize.height(); x++)
            {
                newInputs.at(
                        getFlatIndex({x, y, z}, {
                            mInputSize.height(), mInputSize.width(), mInputSize.depth()
                        })) = 
                    mInputs.at(
                            getFlatIndex({mInputSize.width() - 1 - y, x, z},
                            mInputSize));
            }
        }
    }

    std::vector<char> newOutputs = mOutputs;
    for (int z = 0; z < mOutputSize.depth(); z++)
    {
        for (int y = 0; y < mOutputSize.width(); y++)
        {
            for (int x = 0; x < mOutputSize.height(); x++)
            {
                newOutputs.at(
                        getFlatIndex({x, y, z}, {
                            mOutputSize.height(), mOutputSize.width(), mOutputSize.depth()
                        })) = 
                    mOutputs.at(
                            getFlatIndex({mOutputSize.width() - 1 - y, x, z},
                            mOutputSize));
            }
        }
    }

    return Rule(newInputs, 
            {
                mInputSize.height(),
                mInputSize.width(),
                mInputSize.depth()
            }, newOutputs,
            {
                mOutputSize.height(),
                mOutputSize.width(),
                mOutputSize.depth()
            }, mInputShifts.size(), mP);
}

bool Rule::operator==(const Rule & aRhs) const
{
    if (mInputSize != aRhs.mInputSize || mOutputSize != aRhs.mOutputSize)
    {
        return false;
    }
    return mInputs == aRhs.mInputs && mOutputs == aRhs.mOutputs;
}

std::ostream &operator<<(std::ostream & os, const Rule & aRule)
{
    os << "RULE DATA" << std::endl;
    os << "Input size : ";
    os << aRule.mInputSize << std::endl;
    os << "Input : ";
    for (auto input : aRule.mInputs)
    {
        os << input << ", ";
    }
    os << std::endl << "InputShifts : " << std::endl;
    for (auto a : aRule.mInputShifts)
    {
        for (auto pos : a)
        {
            os << pos << ", ";
        }
        os << std::endl;
    }
    os << std::endl;
    os << "Output size : ";
    os << aRule.mOutputSize << std::endl;
    os << "Output : ";
    for (auto output : aRule.mOutputs)
    {
        os << (int)output << ", ";
    }

    os << std::endl << "OutputShifts : " << std::endl;
    for (auto a : aRule.mOutputShifts)
    {
        for (auto pos : a)
        {
            os << pos << ", ";
        }
        os << std::endl;
    }

    return os;
}

}
}
