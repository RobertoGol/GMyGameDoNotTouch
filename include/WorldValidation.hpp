#pragma once

#include <string>
#include <vector>

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

bool LooksLikeRegistryReference(const std::string& value);
std::vector<ValidationIssue> ValidateWorldForRuntime(const World& world);
bool HasBlockingValidationIssues(const std::vector<ValidationIssue>& issues);
int CountValidationErrors(const std::vector<ValidationIssue>& issues);
int CountValidationWarnings(const std::vector<ValidationIssue>& issues);
std::string BuildValidationSummary(const std::vector<ValidationIssue>& issues);

} // namespace bunker
