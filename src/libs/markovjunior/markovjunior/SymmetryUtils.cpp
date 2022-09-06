#include "SymmetryUtils.h"

namespace ad {
namespace markovjunior {
    // see: https://en.wikipedia.org/wiki/Examples_of_groups#dihedral_group_of_order_8
    const std::map<std::string, SymmetryGroup> gSquareSubgroups = {
        // in order: e     b      a      ba     a2     ba2    a3     ba3
        {"()",      {true, false, false, false, false, false, false, false}},
        {"(x)",     {true, true , false, false, false, false, false, false}},
        {"(y)",     {true, false, false, false, false, true , false, false}},
        {"(x)(y)",  {true, true , false, false, false, true , false, false}},
        {"(xy+)",   {true, false, true , false, true , false, true , false}},
        {"(xy)",    {true, true , true , true , true , true , true , true }},
    };

    const SymmetryGroup gDefaultSquareGroup = gSquareSubgroups.at("(xy)");


    SymmetryGroup getSymmetry(const std::string & aSymmetryString, const SymmetryGroup & aDefaultGroup)
    {
        return !aSymmetryString.compare("") == 0 ?
            gSquareSubgroups.at(aSymmetryString) :
            aDefaultGroup;
    }
}
}
