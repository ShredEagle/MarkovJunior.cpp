#pragma once
#include <array>
#include <map>
#include <string>
#include <vector>
#include <algorithm>

namespace ad {
namespace markovjunior {
typedef std::array<bool, 8> SymmetryGroup;

extern const std::map<std::string, SymmetryGroup> gSquareSubgroups;
extern const SymmetryGroup gDefaultSquareGroup;

// TODO Should crash if does not contains and should not be called if symmetryString is
// null
SymmetryGroup getSymmetry(const std::string & symmetryString,
                          const SymmetryGroup & aDefaultGroup = gDefaultSquareGroup);

template <class T_rule>
std::vector<T_rule> createAllSquareSymmetries(T_rule aRule, SymmetryGroup aSubGroup)
{
    std::vector<T_rule> possibleSymmetricRules;

    possibleSymmetricRules.push_back(aRule);
    possibleSymmetricRules.push_back(possibleSymmetricRules.at(0).reflect());
    possibleSymmetricRules.push_back(possibleSymmetricRules.at(0).rotate());
    possibleSymmetricRules.push_back(possibleSymmetricRules.at(2).reflect());
    possibleSymmetricRules.push_back(possibleSymmetricRules.at(2).rotate());
    possibleSymmetricRules.push_back(possibleSymmetricRules.at(4).reflect());
    possibleSymmetricRules.push_back(possibleSymmetricRules.at(4).rotate());
    possibleSymmetricRules.push_back(possibleSymmetricRules.at(6).reflect());

    std::vector<T_rule> allowedSymmetricRules;

    int i = 0;
    for (T_rule rule : possibleSymmetricRules) {
        if (aSubGroup.at(i++)
            && std::find(allowedSymmetricRules.begin(), allowedSymmetricRules.end(),
                         rule)
                   == allowedSymmetricRules.end()) {
            allowedSymmetricRules.push_back(rule);
        }
    }

    return allowedSymmetricRules;
}
} // namespace markovjunior
} // namespace ad
