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

const std::map<std::string, ad::math::hdr::Rgb<float>> gColorMatching = {
    {"B", ad::math::hdr::gBlack<float>},
    {"W", ad::math::hdr::Rgb<float>{1.f, 1.f, 1.f}},
    {"t", ad::math::hdr::Rgb<float>{0.2f, 0.24f, 0.15f}},
    {"K", ad::math::hdr::Rgb<float>{1.f, 0.35f, 0.6f}},
    {"O", ad::math::hdr::Rgb<float>{1.f, 0.6f, 0.f}},
    {"E", ad::math::hdr::Rgb<float>{0.f, 0.5f, 0.3f}},
    {"D", ad::math::hdr::Rgb<float>{0.4f, 0.43f, 0.37f}},
    {"N", ad::math::hdr::Rgb<float>{0.7f, 0.41f, 0.2f}},
    {"R", ad::math::hdr::gRed<float>},
    {"C", ad::math::hdr::gCyan<float>},
    {"G", ad::math::hdr::Rgb<float>{0.f, 0.9f, 0.2f}},
    {"U", ad::math::hdr::Rgb<float>{0.2f, 0.7f, 1.f}},
    {"P", ad::math::hdr::gMagenta<float>},
    {"Y", ad::math::hdr::Rgb<float>{1.f, 0.9f, 0.2f}},
    {"A", ad::math::hdr::Rgb<float>{0.6f, 0.6f, 0.64f}},
    {"I", ad::math::hdr::Rgb<float>{0.1f, 0.16f, 0.4f}},
    {"F", ad::math::hdr::Rgb<float>{1.f, 0.8f, 0.6f}},
    {"T", ad::math::hdr::Rgb<float>{0.15f, 0.5f, 0.78f}},
    {"S", ad::math::hdr::Rgb<float>{0.8f, 0.8f, 0.6f}},
    {"s", ad::math::hdr::Rgb<float>{0.9f, 0.9f, 0.67f}},
    {"u", ad::math::hdr::Rgb<float>{0.f, 0.3f, 0.7f}},
    {"*", ad::math::hdr::Rgb<float>{0.f, 0.3f, 0.7f}}
};

inline unsigned char convertRgbToUnsignedChar(const math::hdr::Rgb<float> & color)
{
    constexpr auto colorMatchingComp = [](const ad::math::hdr::Rgb<float> & aLhs, const ad::math::hdr::Rgb<float> & aRhs)
    {
        return aLhs.r() + aLhs.g() + aLhs.b() > aRhs.r() + aRhs.g() + aRhs.b();
    };

    static std::map<ad::math::hdr::Rgb<float>, std::string, decltype(colorMatchingComp)> inverseColorMatching(colorMatchingComp);

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
                ruleInput.at(getFlatIndex({x, y, z}, resultSize)) = convertRgbToUnsignedChar(arte::to_hdr(image).at(math::Position<2, int>{x, y}));
                ruleOutput.at(getFlatIndex({x, y, z}, resultSize)) = convertRgbToUnsignedChar(arte::to_hdr(image).at(math::Position<2, int>{x + resultSize.width(), y}));
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
