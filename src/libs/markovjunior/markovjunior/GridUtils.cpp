#include "GridUtils.h"
#include "markovjunior/ImageHelpers.h"

namespace ad {
namespace markovjunior {

math::hdr::Rgb<float>
getColorfromValue(const Grid & aGrid, unsigned char aValue)
{
    unsigned char gridChar =
        (aValue == 255) ? "*"[0] : aGrid.mCharacters.at(aValue);
    std::string gridCharString =
        std::string(1, static_cast<char>(gridChar));
    return gColorMatching.contains(gridCharString)
               ? gColorMatching.at(gridCharString)
               : ad::math::hdr::Rgb<float>{0.f, 0.f, 0.f};
}

}
} // namespace ad
