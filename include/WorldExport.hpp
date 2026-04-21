#pragma once

#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "World.hpp"
#include "WorldValidation.hpp"

namespace bunker {

struct ValidationIssueTally {
    std::string code;
    int count = 0;
};

struct ValidationBaselineIssueEntry {
    ValidationSeverity severity = ValidationSeverity::Warning;
    std::string code;
    std::string objectId;
    std::string scriptTag;
    std::string relatedValue;
};

struct ValidationBaselineSnapshot {
    bool loaded = false;
    std::filesystem::path path{};
    std::string generatedAt;
    std::string targetPath;
    std::string policyLabel;
    std::string decisionLabel;
    int errorCount = 0;
    int warningCount = 0;
    int autoCreatedSemanticAnchorCount = 0;
    std::vector<ValidationIssueTally> issueCounts{};
    std::vector<ValidationBaselineIssueEntry> issueEntries{};
};

struct ValidationBaselineDeltaEntry {
    std::string code;
    int baselineCount = 0;
    int currentCount = 0;
    int delta = 0;
};

struct ValidationBaselineIssueDeltaEntry {
    ValidationSeverity severity = ValidationSeverity::Warning;
    std::string code;
    std::string objectId;
    std::string scriptTag;
    std::string relatedValue;
    int occurrences = 1;
};

struct ValidationBaselineDelta {
    bool hasBaseline = false;
    std::filesystem::path baselinePath{};
    int baselineErrorCount = 0;
    int currentErrorCount = 0;
    int baselineWarningCount = 0;
    int currentWarningCount = 0;
    int baselineAutoCreatedSemanticAnchorCount = 0;
    int currentAutoCreatedSemanticAnchorCount = 0;
    std::vector<ValidationBaselineDeltaEntry> regressions{};
    std::vector<ValidationBaselineDeltaEntry> improvements{};
    std::vector<ValidationBaselineIssueDeltaEntry> issueRegressions{};
    std::vector<ValidationBaselineIssueDeltaEntry> issueImprovements{};
};

struct WorldExportHistoryEntry {
    std::string generatedAt;
    std::string targetPath;
    std::string policyLabel;
    std::string decisionLabel;
    int errorCount = 0;
    int warningCount = 0;
    int autoCreatedSemanticAnchorCount = 0;
    bool baselineUpdated = false;
    std::filesystem::path validationReportPath{};
    std::filesystem::path validationSnapshotPath{};
    std::filesystem::path baselineSnapshotPath{};
    std::string message;
};

enum class WorldExportHistoryFilter {
    All,
    Successful,
    Blocked,
    SaveFailed,
    PrototypeOnly,
    ShippingOnly,
    BaselineUpdatedOnly
};

enum class WorldExportComparePreset {
    LatestSuccessfulShipping,
    LatestSuccessfulPrototype,
    LatestBlocked,
    LatestBaselineUpdated,
    ManualSelection
};

struct WorldExportHistoryQuery {
    WorldExportHistoryFilter filter = WorldExportHistoryFilter::All;
    bool requireSuccessful = false;
    bool requireValidationSnapshot = false;
};

struct WorldExportHistorySelection {
    bool found = false;
    int historyIndex = -1;
    WorldExportComparePreset preset = WorldExportComparePreset::ManualSelection;
    WorldExportHistoryQuery query{};
    std::string summaryLabel;
    std::string badgeLabel;
    std::string fallbackMessage;
};

struct WorldExportResult {
    bool ok = false;
    bool blockedByValidation = false;
    bool blockedByPolicy = false;
    bool saveFailed = false;
    bool baselineUpdated = false;
    ExportValidationPolicy policy = ExportValidationPolicy::AllowWarnings;
    int errorCount = 0;
    int warningCount = 0;
    int autoCreatedSemanticAnchorCount = 0;
    std::filesystem::path worldPath{};
    std::filesystem::path validationReportPath{};
    std::filesystem::path auditTrailPath{};
    std::filesystem::path validationSnapshotPath{};
    std::filesystem::path baselineSnapshotPath{};
    std::string generatedAt;
    std::string message;
};

const char* ExportValidationPolicyLabel(ExportValidationPolicy policy);
const char* WorldExportDecisionLabel(const WorldExportResult& result);
const char* WorldExportHistoryFilterLabel(WorldExportHistoryFilter filter);
const char* WorldExportComparePresetLabel(WorldExportComparePreset preset);
std::filesystem::path ValidationReportPathForWorld(const std::filesystem::path& worldPath);
std::filesystem::path ExportAuditTrailPathForWorld(const std::filesystem::path& worldPath);
std::filesystem::path ValidationSnapshotArchivePathForWorld(const std::filesystem::path& worldPath, std::string_view exportToken);
std::filesystem::path ValidationBaselinePathForWorld(const std::filesystem::path& worldPath);
std::string LoadTextArtifactPreview(const std::filesystem::path& path, std::size_t maxChars = 4096);
bool LoadWorldExportHistory(const std::filesystem::path& worldPath, std::vector<WorldExportHistoryEntry>& entries);
bool MatchesHistoryQuery(const WorldExportHistoryEntry& entry, const WorldExportHistoryQuery& query);
std::vector<WorldExportHistorySelection> FilterWorldExportHistoryEntries(
    const std::vector<WorldExportHistoryEntry>& entries,
    const WorldExportHistoryQuery& query);
WorldExportHistorySelection FindLatestMatchingHistoryEntry(
    const std::vector<WorldExportHistoryEntry>& entries,
    const WorldExportHistoryQuery& query);
WorldExportHistorySelection ResolveComparePresetTarget(
    const std::vector<WorldExportHistoryEntry>& entries,
    WorldExportComparePreset preset,
    int manualSelectionIndex = -1);
std::string BuildWorldExportHistoryBadgeLabel(const WorldExportHistoryEntry& entry);
std::string SummarizeWorldExportHistoryEntry(const WorldExportHistoryEntry& entry);
std::string BuildValidationBaselineSnapshot(const std::vector<ValidationIssue>& issues, const WorldExportResult& result);
bool LoadValidationBaselineSnapshot(const std::filesystem::path& path, ValidationBaselineSnapshot& snapshot);
ValidationBaselineDelta CompareValidationToSnapshot(const std::vector<ValidationIssue>& issues, const std::filesystem::path& snapshotPath);
ValidationBaselineDelta CompareValidationToBaseline(const std::vector<ValidationIssue>& issues, const std::filesystem::path& worldPath);
std::string BuildValidationSnapshotDeltaReport(const ValidationBaselineDelta& delta, std::string_view label);
std::string BuildValidationBaselineDeltaReport(const ValidationBaselineDelta& delta);
std::string BuildWorldValidationReport(
    const World& world,
    const std::vector<ValidationIssue>& issues,
    const WorldExportResult& result);
WorldExportResult ExportWorldWithValidation(const World& world,
    const std::filesystem::path& path,
    ExportValidationPolicy policy = ExportValidationPolicy::AllowWarnings);

}  // namespace bunker
