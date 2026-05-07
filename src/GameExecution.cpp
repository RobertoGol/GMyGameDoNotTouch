#include "../include/GameExecution.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace bunker {

namespace {

std::vector<std::string> CandidateKeysForObject(const MapObject& object) {
    std::vector<std::string> keys;
    auto pushKey = [&](std::string_view value) {
        const std::string normalized = NormalizeResourceLookupKey(value);
        if (!normalized.empty() &&
            std::find(keys.begin(), keys.end(), normalized) == keys.end()) {
            keys.push_back(normalized);
        }
    };

    pushKey(object.registryId);
    pushKey(object.prefabSourceId);
    pushKey(object.scriptTag);
    pushKey(object.displayName);
    return keys;
}

bool IsRenderableAssetExtension(std::string_view extension) {
    return extension == ".dds" || extension == ".nif" || extension == ".bgsm" || extension == ".bgem";
}

bool IsPluginExtension(std::string_view extension) {
    return extension == ".esm" || extension == ".esp" || extension == ".esl";
}

bool IsScriptExtension(std::string_view extension) {
    return extension == ".pex" || extension == ".psc";
}

}  // namespace

std::string NormalizeResourceLookupKey(std::string_view value) {
    std::string normalized;
    normalized.reserve(value.size());
    for (const unsigned char ch : value) {
        if (std::isalnum(ch)) {
            normalized.push_back(static_cast<char>(std::tolower(ch)));
        }
    }
    return normalized;
}

void GlobalResourceManager::Rebuild(const ExternalDataScanSummary& summary) {
    resourcesByKey_.clear();
    for (const auto& file : summary.files) {
        const std::string stemKey = NormalizeResourceLookupKey(file.path.stem().string());
        if (!stemKey.empty()) {
            resourcesByKey_[stemKey].push_back(file.path);
        }
        const std::string fileKey = NormalizeResourceLookupKey(file.fileName);
        if (!fileKey.empty() && fileKey != stemKey) {
            resourcesByKey_[fileKey].push_back(file.path);
        }
    }
}

std::vector<std::filesystem::path> GlobalResourceManager::FindAll(std::string_view key) const {
    const std::string normalized = NormalizeResourceLookupKey(key);
    if (normalized.empty()) {
        return {};
    }
    const auto it = resourcesByKey_.find(normalized);
    return it == resourcesByKey_.end() ? std::vector<std::filesystem::path>{} : it->second;
}

std::filesystem::path GlobalResourceManager::FindFirstWithExtension(std::string_view key, std::string_view extension) const {
    std::string normalizedExtension(extension);
    std::transform(normalizedExtension.begin(), normalizedExtension.end(), normalizedExtension.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    for (const auto& candidate : FindAll(key)) {
        std::string candidateExtension = candidate.extension().string();
        std::transform(candidateExtension.begin(), candidateExtension.end(), candidateExtension.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        if (candidateExtension == normalizedExtension) {
            return candidate;
        }
    }
    return {};
}

std::filesystem::path GlobalResourceManager::FindBestAssetFor(const MapObject& object) const {
    for (const auto& key : CandidateKeysForObject(object)) {
        const auto candidates = FindAll(key);
        for (const auto& candidate : candidates) {
            const std::string extension = candidate.extension().string();
            if (IsRenderableAssetExtension(extension)) {
                return candidate;
            }
        }
    }
    return {};
}

void DependencyLinker::Rebuild(const ExternalDataScanSummary& summary) {
    pluginByKey_.clear();
    for (const auto& file : summary.files) {
        if (!IsPluginExtension(file.extension)) {
            continue;
        }
        const std::string key = NormalizeResourceLookupKey(file.path.stem().string());
        if (!key.empty()) {
            pluginByKey_[key] = file.path;
        }
    }
}

std::filesystem::path DependencyLinker::ResolvePluginProxyFor(const MapObject& object) const {
    for (const auto& key : CandidateKeysForObject(object)) {
        const auto it = pluginByKey_.find(key);
        if (it != pluginByKey_.end()) {
            return it->second;
        }
    }
    return {};
}

WorldExecutionContext BuildWorldExecutionContext(const World& world, const std::filesystem::path& scanRoot) {
    WorldExecutionContext context;
    context.externalData = ScanExportDataDirectory(scanRoot);
    context.resources.Rebuild(context.externalData);
    context.dependencyLinker.Rebuild(context.externalData);

    context.objects.reserve(world.objects.size());
    for (int objectIndex = 0; objectIndex < static_cast<int>(world.objects.size()); ++objectIndex) {
        const auto& object = world.objects[static_cast<std::size_t>(objectIndex)];
        GameObjectInstance instance;
        instance.registryId = object.registryId;
        instance.worldObjectIndex = objectIndex;
        instance.renderResourcePath = context.resources.FindBestAssetFor(object);
        instance.pluginProxyPath = context.dependencyLinker.ResolvePluginProxyFor(object);
        if (!object.scriptTag.empty()) {
            instance.compiledScriptPath = context.resources.FindFirstWithExtension(object.scriptTag, ".pex");
            instance.sourceScriptPath = context.resources.FindFirstWithExtension(object.scriptTag, ".psc");
            instance.hasTerminalOverlay = object.interaction == InteractionType::Terminal;
        }

        GameComponent physics;
        physics.kind = GameComponentKind::Physics;
        physics.width = object.width;
        physics.depth = object.depth;
        physics.height = object.height;
        physics.blocksMovement = object.blocksMovement;
        instance.components.push_back(std::move(physics));

        if (!instance.renderResourcePath.empty()) {
            GameComponent render;
            render.kind = GameComponentKind::Render;
            render.resourcePath = instance.renderResourcePath;
            instance.components.push_back(std::move(render));
        }

        if (!object.lootEntries.empty()) {
            GameComponent inventory;
            inventory.kind = GameComponentKind::Inventory;
            inventory.inventoryTemplate = object.lootEntries;
            instance.components.push_back(std::move(inventory));
        }

        context.objects.push_back(std::move(instance));
    }

    return context;
}

const GameObjectInstance* FindGameObjectInstance(const WorldExecutionContext& context, std::string_view registryId) {
    const auto it = std::find_if(context.objects.begin(), context.objects.end(), [&](const GameObjectInstance& instance) {
        return instance.registryId == registryId;
    });
    return it == context.objects.end() ? nullptr : &*it;
}

std::string DescribeInteractionOverlay(const MapObject& object, const WorldExecutionContext& context) {
    std::ostringstream description;
    description << object.displayName;
    if (!object.scriptTag.empty()) {
        description << "\nscriptTag: " << object.scriptTag;
    }
    if (!object.linkTarget.empty()) {
        description << "\nlinkTarget: " << object.linkTarget;
    }
    if (const auto* instance = FindGameObjectInstance(context, object.registryId); instance != nullptr) {
        if (!instance->renderResourcePath.empty()) {
            description << "\nasset: " << instance->renderResourcePath.filename().string();
        }
        if (!instance->pluginProxyPath.empty()) {
            description << "\nproxy plugin: " << instance->pluginProxyPath.filename().string();
        }
        if (!instance->compiledScriptPath.empty()) {
            description << "\ncompiled script: " << instance->compiledScriptPath.filename().string();
        } else if (!instance->sourceScriptPath.empty()) {
            description << "\nscript source: " << instance->sourceScriptPath.filename().string();
        }
    }
    return description.str();
}

bool TryExecuteCompiledScript(const MapObject& object, const WorldExecutionContext& context, std::string& statusText) {
    if (object.scriptTag.empty()) {
        return false;
    }
    const auto* instance = FindGameObjectInstance(context, object.registryId);
    if (instance == nullptr) {
        return false;
    }
    if (!instance->compiledScriptPath.empty()) {
        statusText = "Executed compiled script bridge for " + object.scriptTag +
            " via " + instance->compiledScriptPath.filename().string() + ".";
        return true;
    }
    if (!instance->sourceScriptPath.empty()) {
        statusText = "Compiled script missing; fell back to source script bridge for " + object.scriptTag +
            " via " + instance->sourceScriptPath.filename().string() + ".";
        return true;
    }
    return false;
}

}  // namespace bunker
