#pragma once

#include <string>
#include <unordered_set>
#include <fstream>
#include "AppPaths.hpp"

namespace bunker {

class StaticEraser {
public:
    void Erase(const std::string& registryId) {
        if (!registryId.empty()) m_erasedIds.insert(registryId);
    }

    bool IsErased(const std::string& registryId) const {
        return m_erasedIds.find(registryId) != m_erasedIds.end();
    }

    void Save(std::string_view worldName) const {
        std::ofstream out(StaticEraserPath(worldName));
        for (const auto& id : m_erasedIds) {
            out << id << "\n";
        }
    }

    void Load(std::string_view worldName) {
        m_erasedIds.clear();
        std::ifstream in(StaticEraserPath(worldName));
        std::string id;
        while (std::getline(in, id)) {
            if (!id.empty()) m_erasedIds.insert(id);
        }
    }

private:
    std::unordered_set<std::string> m_erasedIds;
};

} // namespace bunker
