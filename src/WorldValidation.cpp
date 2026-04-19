#include "../include/WorldValidation.hpp"

#include <unordered_set>

#include "../include/GameplayDescriptorRegistry.hpp"

namespace bunker {

bool LooksLikeRegistryReference(const std::string& value) {
    return value.size() >= 2 && value.front() == '[' && value.back() == ']';
}

std::vector<ValidationIssue> ValidateWorldForRuntime(const World& world) {
    std::vector<ValidationIssue> issues;
    std::unordered_set<std::string> registryIds;

    for (const auto& obj : world.objects) {
        if (obj.registryId.empty()) {
            issues.push_back({
                ValidationSeverity::Error,
                "missing_registry_id",
                "",
                "Object has empty registryId."
            });
        } else if (!registryIds.insert(obj.registryId).second) {
            issues.push_back({
                ValidationSeverity::Error,
                "duplicate_registry_id",
                obj.registryId,
                "Duplicate registryId detected: " + obj.registryId
            });
        }

        if (obj.displayName.empty()) {
            issues.push_back({
                ValidationSeverity::Warning,
                "missing_display_name",
                obj.registryId,
                "Object has empty displayName."
            });
        }

        const bool linkLooksLikeRegistryRef = LooksLikeRegistryReference(obj.linkTarget);
        if (!obj.linkTarget.empty() && linkLooksLikeRegistryRef && !world.HasObject(obj.linkTarget)) {
            issues.push_back({
                ValidationSeverity::Error,
                "broken_link_target",
                obj.registryId,
                "linkTarget does not resolve to any object: " + obj.linkTarget
            });
        }

        const std::string_view normalizedTag = NormalizeGameplayDescriptorTag(obj.scriptTag);
        if (normalizedTag != obj.scriptTag && !obj.scriptTag.empty()) {
            issues.push_back({
                ValidationSeverity::Warning,
                "legacy_script_tag_alias",
                obj.registryId,
                "scriptTag '" + obj.scriptTag + "' is a legacy alias. Prefer '" + std::string(normalizedTag) + "'."
            });
        }

        if (const auto* spec = FindGameplayDescriptor(obj.scriptTag)) {
            if (obj.interaction != spec->preferredInteraction) {
                issues.push_back({
                    ValidationSeverity::Warning,
                    "interaction_mismatch",
                    obj.registryId,
                    "Interaction does not match preferred interaction for scriptTag '" +
                        std::string(spec->scriptTag) + "'."
                });
            }

            if (obj.category != spec->preferredCategory) {
                issues.push_back({
                    ValidationSeverity::Warning,
                    "category_mismatch",
                    obj.registryId,
                    "Category does not match preferred category for scriptTag '" +
                        std::string(spec->scriptTag) + "'."
                });
            }

            if (spec->requiresLinkTarget && obj.linkTarget.empty()) {
                issues.push_back({
                    ValidationSeverity::Error,
                    "missing_required_link_target",
                    obj.registryId,
                    "scriptTag '" + std::string(spec->scriptTag) + "' requires a linkTarget."
                });
            }
        } else if (!obj.scriptTag.empty() && ScriptTagRequiresLinkTarget(obj.scriptTag) && obj.linkTarget.empty()) {
            issues.push_back({
                ValidationSeverity::Error,
                "missing_required_link_target",
                obj.registryId,
                "scriptTag requires a linkTarget."
            });
        } else if (!obj.scriptTag.empty()) {
            issues.push_back({
                ValidationSeverity::Warning,
                "unknown_script_tag",
                obj.registryId,
                "Unknown scriptTag: '" + obj.scriptTag + "'."
            });
        }

        if (obj.interaction == InteractionType::Transition && obj.linkTarget.empty()) {
            issues.push_back({
                ValidationSeverity::Warning,
                "transition_without_link_target",
                obj.registryId,
                "Transition object has no linkTarget."
            });
        }
    }

    return issues;
}

int CountValidationErrors(const std::vector<ValidationIssue>& issues) {
    int count = 0;
    for (const auto& issue : issues) {
        if (issue.severity == ValidationSeverity::Error) {
            ++count;
        }
    }
    return count;
}

int CountValidationWarnings(const std::vector<ValidationIssue>& issues) {
    int count = 0;
    for (const auto& issue : issues) {
        if (issue.severity == ValidationSeverity::Warning) {
            ++count;
        }
    }
    return count;
}

std::string BuildValidationSummary(const std::vector<ValidationIssue>& issues) {
    const int errors = CountValidationErrors(issues);
    const int warnings = CountValidationWarnings(issues);

    if (errors == 0 && warnings == 0) {
        return "World validation passed: no issues found.";
    }
    if (errors == 0) {
        return "World validation passed with warnings: " + std::to_string(warnings) + ".";
    }
    return "World validation failed: " + std::to_string(errors) +
           " errors, " + std::to_string(warnings) + " warnings.";
}

} // namespace bunker
