#pragma once
#include <map>
#include "../../include/Registry_ID.hpp"

namespace BunkerProtocol {

    struct EditorAsset {
        char typeID;
        std::string name;
        EntityType category;
    };

    class UIObjectWindow {
    private:
        std::vector<EditorAsset> library;

    public:
        void Init() {
            // Заполняем библиотеку из реестра
            library.push_back({1, "Bunker_Wall_Straight", EntityType::Structure});
            library.push_back({2, "Bunker_Door_Auto", EntityType::Structure});
            library.push_back({3, "Medical_Crate_01", EntityType::Item});
        }

        // Здесь будет логика отрисовки дерева категорий (Tree Node)
    };
}
