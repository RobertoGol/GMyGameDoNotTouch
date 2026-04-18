#pragma once
#include <vector>
#include <string>
#include <cstdlib>

enum class LootTier {
    Common,
    Rare,
    SR,
    Legendary
};

class LootGenerator {
public:
    static std::vector<std::string> Generate() {
        std::vector<std::string> loot;

        int count = 3 + rand() % 3;

        for (int i = 0; i < count; i++) {
            int roll = rand() % 100;

            if (roll < 60) loot.push_back("junk");
            else if (roll < 90) loot.push_back("rare");
            else loot.push_back("sr");
        }

        return loot;
    }
};