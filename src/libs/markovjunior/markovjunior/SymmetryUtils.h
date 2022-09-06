#pragma once
#include <algorithm>
#include <array>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace ad {
namespace markovjunior
{
    typedef std::array<bool, 8> SymmetryGroup;

    extern const std::map<std::string, SymmetryGroup> gSquareSubgroups;
    extern const SymmetryGroup gDefaultSquareGroup;

    // TODO Should crash if does not contains and should not be called if symmetryString is null
    SymmetryGroup getSymmetry(const std::string & symmetryString, const SymmetryGroup & aDefaultGroup = gDefaultSquareGroup );

    template<class T>
    concept IsSymmetrizable = requires(T t) {
        requires std::equality_comparable<T>;
        {t.reflect()} -> std::convertible_to<T>;
        {t.rotate()} -> std::convertible_to<T>;
    };

    template<IsSymmetrizable T_rule>
    std::vector<T_rule> createAllSquareSymmetries(T_rule aRule, SymmetryGroup aSubGroup)
    {
        std::array<std::optional<T_rule>, 8> possibleSymmetricRules;

        possibleSymmetricRules.at(0) = aRule;
        possibleSymmetricRules.at(1) = 
            aSubGroup.at(1) && possibleSymmetricRules.at(0) ? std::optional<T_rule>(possibleSymmetricRules.at(0).value().reflect()) : std::nullopt;
        possibleSymmetricRules.at(2) = 
            aSubGroup.at(2) && possibleSymmetricRules.at(0) ? std::optional<T_rule>(possibleSymmetricRules.at(0).value().rotate()) : std::nullopt;
        possibleSymmetricRules.at(3) = 
            aSubGroup.at(3) && possibleSymmetricRules.at(2) ? std::optional<T_rule>(possibleSymmetricRules.at(2).value().reflect()) : std::nullopt;
        possibleSymmetricRules.at(4) = 
            aSubGroup.at(4) && possibleSymmetricRules.at(2) ? std::optional<T_rule>(possibleSymmetricRules.at(2).value().rotate()) : std::nullopt;
        possibleSymmetricRules.at(5) = 
            aSubGroup.at(5) && possibleSymmetricRules.at(4) ? std::optional<T_rule>(possibleSymmetricRules.at(4).value().reflect()) : std::nullopt;
        possibleSymmetricRules.at(6) = 
            aSubGroup.at(6) && possibleSymmetricRules.at(4) ? std::optional<T_rule>(possibleSymmetricRules.at(4).value().rotate()) : std::nullopt;
        possibleSymmetricRules.at(7) = 
            aSubGroup.at(7) && possibleSymmetricRules.at(6) ? std::optional<T_rule>(possibleSymmetricRules.at(6).value().reflect()) : std::nullopt;

        std::vector<T_rule> allowedSymmetricRules;

        for (std::optional<T_rule> rule : possibleSymmetricRules)
        {
            if (rule && std::find(allowedSymmetricRules.begin(), allowedSymmetricRules.end(), rule.value()) == allowedSymmetricRules.end())
            {
                allowedSymmetricRules.push_back(rule.value());
            }
        }

        return allowedSymmetricRules;
    }
} // namespace markov
} // namespace ad
