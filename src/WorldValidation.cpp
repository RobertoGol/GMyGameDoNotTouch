#include "../include/WorldValidation.hpp"

#include <unordered_set>

#include "../include/GameplayDescriptorRegistry.hpp"
#include "../include/WorldSemanticAuthoring.hpp"

namespace bunker {

namespace {

bool HasSemanticAnchor(const World& world, std::string_view scriptTag) {
    return world.FindObjectByScriptTag(std::string(scriptTag)) != nullptr;
}

void AddMissingDependencyIssue(std::vector<ValidationIssue>& issues,
                               const MapObject& object,
                               std::string_view scriptTag,
                               std::string_view dependencyTag) {
    issues.push_back({
        ValidationSeverity::Warning,
        "missing_authored_dependency",
        object.registryId,
        "scriptTag '" + std::string(scriptTag) + "' is authored without required anchor '" +
            std::string(dependencyTag) + "' in this world.",
        std::string(scriptTag),
        std::string(dependencyTag)
    });
}

void ValidateSemanticDependencies(const World& world,
                                  const MapObject& object,
                                  std::string_view scriptTag,
                                  std::vector<ValidationIssue>& issues) {
    auto require = [&](std::string_view dependencyTag) {
        if (!HasSemanticAnchor(world, dependencyTag)) {
            AddMissingDependencyIssue(issues, object, scriptTag, dependencyTag);
        }
    };

    for (const std::string_view dependencyTag : RequiredSemanticDependencyTags(scriptTag)) {
        require(dependencyTag);
    }
}

void AddAutoCreatedSemanticAnchorIssue(const MapObject& object, std::vector<ValidationIssue>& issues) {
    std::string message = "Semantic anchor was auto-created and still needs authoring adoption before ship/export.";
    if (IsPinnedSemanticAnchor(object)) {
        message = "Semantic anchor was auto-created, then pinned in place. Adopt it as authored before ship/export.";
    }

    issues.push_back({
        ValidationSeverity::Warning,
        "auto_created_semantic_anchor",
        object.registryId,
        message,
        object.scriptTag,
        IsPinnedSemanticAnchor(object) ? "pinned" : "auto"
    });
}

}  // namespace

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

            const char* defaultLinkTarget = DefaultGameplayDescriptorLinkTarget(spec->scriptTag);
            if (defaultLinkTarget != nullptr) {
                if (obj.linkTarget.empty() && !spec->requiresLinkTarget) {
                    issues.push_back({
                        ValidationSeverity::Warning,
                        "missing_canonical_link_target",
                        obj.registryId,
                        "scriptTag '" + std::string(spec->scriptTag) + "' is missing its canonical linkTarget '" +
                            std::string(defaultLinkTarget) + "'.",
                        std::string(spec->scriptTag),
                        std::string(defaultLinkTarget)
                    });
                } else if (!obj.linkTarget.empty() &&
                           !linkLooksLikeRegistryRef &&
                           obj.linkTarget != defaultLinkTarget) {
                    issues.push_back({
                        ValidationSeverity::Warning,
                        "descriptor_link_target_mismatch",
                        obj.registryId,
                        "scriptTag '" + std::string(spec->scriptTag) + "' uses linkTarget '" + obj.linkTarget +
                            "' instead of canonical '" + std::string(defaultLinkTarget) + "'.",
                        std::string(spec->scriptTag),
                        std::string(defaultLinkTarget)
                    });
                }
            }

            if (spec->requiresLinkTarget && obj.linkTarget.empty()) {
                issues.push_back({
                    ValidationSeverity::Error,
                    "missing_required_link_target",
                    obj.registryId,
                    "scriptTag '" + std::string(spec->scriptTag) + "' requires a linkTarget.",
                    std::string(spec->scriptTag),
                    defaultLinkTarget != nullptr ? std::string(defaultLinkTarget) : std::string()
                });
            }

            ValidateSemanticDependencies(world, obj, spec->scriptTag, issues);
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

        if (obj.semanticAutoCreated) {
            AddAutoCreatedSemanticAnchorIssue(obj, issues);
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

bool HasBlockingValidationIssues(const std::vector<ValidationIssue>& issues) {
    for (const auto& issue : issues) {
        if (issue.severity == ValidationSeverity::Error) {
            return true;
        }
    }
    return false;
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

int CountValidationIssuesByCode(const std::vector<ValidationIssue>& issues, std::string_view code) {
    int count = 0;
    for (const auto& issue : issues) {
        if (issue.code == code) {
            ++count;
        }
    }
    return count;
}

bool ExportPolicyBlocksWorld(const std::vector<ValidationIssue>& issues, ExportValidationPolicy policy, std::string& reason) {
    if (policy == ExportValidationPolicy::BlockAutoCreatedSemanticAnchors) {
        const int autoCreatedAnchorCount = CountValidationIssuesByCode(issues, "auto_created_semantic_anchor");
        if (autoCreatedAnchorCount > 0) {
            reason = "auto-created semantic anchors still present: " + std::to_string(autoCreatedAnchorCount);
            return true;
        }
    }

    reason.clear();
    return false;
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
