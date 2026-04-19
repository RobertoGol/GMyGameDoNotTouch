#pragma once

#include <string>
#include <unordered_set>
#include <vector>

#include "GameplayDescriptorRegistry.hpp"
#include "World.hpp"

namespace bunker {

enum class ValidationSeverity {
    Warning,
    Error
};

struct ValidationIssue {
    ValidationSeverity severity = ValidationSeverity::Warning;
    std::string code;
    std::string objectId;
    std::string message;
};

inline std::vector<ValidationIssue> ValidateWorldForRuntime(const World& world) {
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

        if (!obj.linkTarget.empty() && !world.HasObject(obj.linkTarget)) {
            issues.push_back({
                ValidationSeverity::Error,
                "broken_link_target",
                obj.registryId,
                "linkTarget does not resolve to any object: " + obj.linkTarget
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

inline int CountValidationErrors(const std::vector<ValidationIssue>& issues) {
    int count = 0;
    for (const auto& issue : issues) {
        if (issue.severity == ValidationSeverity::Error) {
            ++count;
        }
    }
    return count;
}

inline int CountValidationWarnings(const std::vector<ValidationIssue>& issues) {
    int count = 0;
    for (const auto& issue : issues) {
        if (issue.severity == ValidationSeverity::Warning) {
            ++count;
        }
    }
    return count;
}

inline std::string BuildValidationSummary(const std::vector<ValidationIssue>& issues) {
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