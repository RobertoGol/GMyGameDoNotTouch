#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "World.hpp"

namespace bunker {

enum class ValidationSeverity {
    Warning,
    Error
};

enum class ExportValidationPolicy {
    AllowWarnings,
    BlockAutoCreatedSemanticAnchors
};

struct ValidationIssue {
    ValidationSeverity severity = ValidationSeverity::Warning;
    std::string code;
    std::string objectId;
    std::string message;
    std::string scriptTag;
    std::string relatedValue;
};

bool LooksLikeRegistryReference(const std::string& value);
std::vector<ValidationIssue> ValidateWorldForRuntime(const World& world);
bool HasBlockingValidationIssues(const std::vector<ValidationIssue>& issues);
int CountValidationErrors(const std::vector<ValidationIssue>& issues);
int CountValidationWarnings(const std::vector<ValidationIssue>& issues);
int CountValidationIssuesByCode(const std::vector<ValidationIssue>& issues, std::string_view code);
bool ExportPolicyBlocksWorld(const std::vector<ValidationIssue>& issues, ExportValidationPolicy policy, std::string& reason);
std::string BuildValidationSummary(const std::vector<ValidationIssue>& issues);

} // namespace bunker
