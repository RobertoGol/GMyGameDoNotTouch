#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "MapObject.hpp"
#include "World.hpp"
#include "WorldExport.hpp"

namespace bunker {

enum class GameComponentKind {
    Render,
    Physics,
    Inventory
};

struct GameComponent {
    GameComponentKind kind = GameComponentKind::Render;
    std::filesystem::path resourcePath{};
    float width = 0.0f;
    float depth = 0.0f;
    float height = 0.0f;
    bool blocksMovement = false;
    std::vector<LootEntry> inventoryTemplate{};
};

struct GameObjectInstance {
    std::string registryId;
    int worldObjectIndex = -1;
    std::vector<GameComponent> components{};
    std::filesystem::path renderResourcePath{};
    std::filesystem::path pluginProxyPath{};
    std::filesystem::path compiledScriptPath{};
    std::filesystem::path sourceScriptPath{};
    bool hasTerminalOverlay = false;
};

class GlobalResourceManager {
public:
    void Rebuild(const ExternalDataScanSummary& summary);
    std::vector<std::filesystem::path> FindAll(std::string_view key) const;
    std::filesystem::path FindFirstWithExtension(std::string_view key, std::string_view extension) const;
    std::filesystem::path FindBestAssetFor(const MapObject& object) const;

private:
    std::unordered_map<std::string, std::vector<std::filesystem::path>> resourcesByKey_{};
};

class DependencyLinker {
public:
    void Rebuild(const ExternalDataScanSummary& summary);
    std::filesystem::path ResolvePluginProxyFor(const MapObject& object) const;

private:
    std::unordered_map<std::string, std::filesystem::path> pluginByKey_{};
};

struct WorldExecutionContext {
    ExternalDataScanSummary externalData{};
    GlobalResourceManager resources{};
    DependencyLinker dependencyLinker{};
    std::vector<GameObjectInstance> objects{};
    int adoptedAnchorCount = 0;
    std::string adoptionStatus;
};

std::string NormalizeResourceLookupKey(std::string_view value);
WorldExecutionContext BuildWorldExecutionContext(const World& world, const std::filesystem::path& scanRoot = {});
const GameObjectInstance* FindGameObjectInstance(const WorldExecutionContext& context, std::string_view registryId);
std::string DescribeInteractionOverlay(const MapObject& object, const WorldExecutionContext& context);
bool TryExecuteCompiledScript(const MapObject& object, const WorldExecutionContext& context, std::string& statusText);

}  // namespace bunker
