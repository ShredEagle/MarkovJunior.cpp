#pragma once

#include "markovjunior/Grid.h"
#include <math/Color.h>
#include <math/Vector.h>
#include <arte/Image.h>

#include <array>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace ad {
namespace markovjunior {

const std::map<std::string, ad::math::sdr::Rgb> gColorMatching = {
    {"B", ad::math::sdr::gBlack},
    {"W", ad::math::sdr::Rgb{0xff, 0xff, 0xff}},
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
    {"A", ad::math::sdr::Rgb{0xc2, 0xc3, 0xc7}},
    {"I", ad::math::sdr::Rgb{0x1d, 0x2b, 0x53}},
    {"F", ad::math::sdr::Rgb{0xff, 0xcc, 0xaa}},
    {"T", ad::math::sdr::Rgb{0x2b, 0x8a, 0xa9}},
    {"S", ad::math::sdr::Rgb{0xd0, 0xd8, 0xac}},
    {"s", ad::math::sdr::Rgb{0xe9, 0xe0, 0xb2}},
    {"u", ad::math::sdr::Rgb{0x06, 0x5a, 0xb5}}
};

inline unsigned char convertRgbToUnsignedChar(const math::sdr::Rgb & color)
{
    constexpr auto colorMatchingComp = [](const ad::math::sdr::Rgb & aLhs, const ad::math::sdr::Rgb & aRhs)
    {
        return aLhs.r() + aLhs.g() + aLhs.b() > aRhs.r() + aRhs.g() + aRhs.b();
    };

    static std::map<ad::math::sdr::Rgb, std::string, decltype(colorMatchingComp)> inverseColorMatching(colorMatchingComp);

    if (inverseColorMatching.empty())
    {
        for (const auto & [name, rgb] : gColorMatching)
        {
            inverseColorMatching.emplace(rgb, name);
        }
    }

    return inverseColorMatching.at(color)[0];
}

inline std::tuple<std::array<std::vector<int>, 2>, math::Size<3, int>> loadRuleFromImage(const filesystem::path & aFilepath)
{
    arte::ImageRgb image(aFilepath, arte::ImageOrientation::Unchanged);
    math::Size<3, int> resultSize = {
        image.dimensions().width() / 2,
        image.dimensions().height(),
        1,
    };
    std::vector<int> ruleInput;
    std::vector<int> ruleOutput;

    for (int z = 0; z < resultSize.depth(); z++)
    {
        for (int y = 0; y < resultSize.height(); y++)
        {
            for (int x = 0; x < resultSize.width(); x++)
            {
                ruleInput.at(getFlatIndex({x, y, z}, resultSize)) = convertRgbToUnsignedChar(image.at(math::Position<2, int>{x, y}));
                ruleOutput.at(getFlatIndex({x, y, z}, resultSize)) = convertRgbToUnsignedChar(image.at(math::Position<2, int>{x + resultSize.width(), y}));
            }
        }
    }

    return {{ruleInput, ruleOutput}, resultSize};
}

inline std::tuple<std::vector<int>, math::Size<3, int>> loadConvChainSample(const filesystem::path & aFilepath)
{
    arte::ImageRgb image(aFilepath, arte::ImageOrientation::Unchanged);
    math::Size<3, int> resultSize = {
        image.dimensions().width(),
        image.dimensions().height(),
        1,
    };
    std::vector<int> ruleSample(image.dimensions().width() * image.dimensions().height(), 0);

    for (int z = 0; z < resultSize.depth(); z++)
    {
        for (int y = 0; y < resultSize.height(); y++)
        {
            for (int x = 0; x < resultSize.width(); x++)
            {
                ruleSample.at(getFlatIndex({x, y, z}, resultSize)) = image.at(math::Position<2, int>{x, y}) == math::sdr::gWhite ? -1 : 0;
            }
        }
    }

    return {ruleSample, resultSize};
}

}
}
