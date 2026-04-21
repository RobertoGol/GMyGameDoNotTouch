#include "../include/WorldExport.hpp"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>

#include "../include/AtomicPersistence.hpp"

namespace bunker {

namespace {

std::string CurrentLocalTimestamp() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t rawTime = std::chrono::system_clock::to_time_t(now);
    std::tm localTime{};
#if defined(_WIN32)
    localtime_s(&localTime, &rawTime);
#else
    localtime_r(&rawTime, &localTime);
#endif

    std::ostringstream formatted;
    formatted << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return formatted.str();
}

std::string CurrentLocalTimestampToken() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t rawTime = std::chrono::system_clock::to_time_t(now);
    std::tm localTime{};
#if defined(_WIN32)
    localtime_s(&localTime, &rawTime);
#else
    localtime_r(&rawTime, &localTime);
#endif

    const auto micros = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()) % 1000000;
    std::ostringstream formatted;
    formatted << std::put_time(&localTime, "%Y%m%d_%H%M%S")
              << '_' << std::setw(6) << std::setfill('0') << micros.count();
    return formatted.str();
}

std::map<std::string, int> CountIssuesByCodeMap(const std::vector<ValidationIssue>& issues) {
    std::map<std::string, int> counts;
    for (const auto& issue : issues) {
        ++counts[issue.code];
    }
    return counts;
}

std::vector<ValidationIssueTally> BuildIssueTallies(const std::vector<ValidationIssue>& issues) {
    std::vector<ValidationIssueTally> tallies;
    for (const auto& [code, count] : CountIssuesByCodeMap(issues)) {
        tallies.push_back({code, count});
    }
    return tallies;
}

std::string TrimLinePrefix(const std::string& line, const std::string& prefix) {
    if (line.rfind(prefix, 0) != 0) {
        return {};
    }
    return line.substr(prefix.size());
}

bool TryParseInt(const std::string& text, int& value) {
    try {
        std::size_t consumed = 0;
        value = std::stoi(text, &consumed);
        return consumed == text.size();
    } catch (...) {
        value = 0;
        return false;
    }
}

bool ParseYesNo(const std::string& text) {
    return text == "yes" || text == "true" || text == "1";
}

const char* BaselineSeverityLabel(ValidationSeverity severity) {
    return severity == ValidationSeverity::Error ? "error" : "warning";
}

ValidationSeverity ParseBaselineSeverityLabel(const std::string& text) {
    return text == "error" ? ValidationSeverity::Error : ValidationSeverity::Warning;
}

std::string BuildIssueSignature(const ValidationBaselineIssueEntry& entry) {
    return std::string(BaselineSeverityLabel(entry.severity)) + "|" +
        entry.code + "|" +
        entry.objectId + "|" +
        entry.scriptTag + "|" +
        entry.relatedValue;
}

ValidationBaselineIssueEntry MakeBaselineIssueEntry(const ValidationIssue& issue) {
    ValidationBaselineIssueEntry entry;
    entry.severity = issue.severity;
    entry.code = issue.code;
    entry.objectId = issue.objectId;
    entry.scriptTag = issue.scriptTag;
    entry.relatedValue = issue.relatedValue;
    return entry;
}

std::vector<ValidationBaselineIssueEntry> BuildBaselineIssueEntries(const std::vector<ValidationIssue>& issues) {
    std::vector<ValidationBaselineIssueEntry> entries;
    entries.reserve(issues.size());
    for (const auto& issue : issues) {
        entries.push_back(MakeBaselineIssueEntry(issue));
    }
    return entries;
}

std::vector<std::string> SplitTabSeparated(const std::string& line) {
    std::vector<std::string> parts;
    std::size_t cursor = 0;
    while (cursor <= line.size()) {
        const std::size_t tab = line.find('\t', cursor);
        if (tab == std::string::npos) {
            parts.push_back(line.substr(cursor));
            break;
        }
        parts.push_back(line.substr(cursor, tab - cursor));
        cursor = tab + 1;
    }
    return parts;
}

std::string FormatBaselineIssueDeltaEntry(const ValidationBaselineIssueDeltaEntry& entry) {
    std::ostringstream formatted;
    formatted << entry.code;
    if (!entry.objectId.empty()) {
        formatted << " :: " << entry.objectId;
    }
    if (!entry.scriptTag.empty()) {
        formatted << " :: scriptTag=" << entry.scriptTag;
    }
    if (!entry.relatedValue.empty()) {
        formatted << " :: related=" << entry.relatedValue;
    }
    if (entry.occurrences > 1) {
        formatted << " (x" << entry.occurrences << ")";
    }
    return formatted.str();
}

ValidationBaselineDelta CompareValidationToLoadedSnapshot(
    const std::vector<ValidationIssue>& issues,
    const ValidationBaselineSnapshot& snapshot,
    const std::filesystem::path& snapshotPath) {
    ValidationBaselineDelta delta;
    delta.hasBaseline = true;
    delta.baselinePath = snapshotPath;
    delta.baselineErrorCount = snapshot.errorCount;
    delta.currentErrorCount = CountValidationErrors(issues);
    delta.baselineWarningCount = snapshot.warningCount;
    delta.currentWarningCount = CountValidationWarnings(issues);
    delta.baselineAutoCreatedSemanticAnchorCount = snapshot.autoCreatedSemanticAnchorCount;
    delta.currentAutoCreatedSemanticAnchorCount = CountValidationIssuesByCode(issues, "auto_created_semantic_anchor");

    std::map<std::string, int> baselineCounts;
    for (const auto& tally : snapshot.issueCounts) {
        baselineCounts[tally.code] = tally.count;
    }
    const auto currentCounts = CountIssuesByCodeMap(issues);

    std::map<std::string, int> allCounts = baselineCounts;
    for (const auto& [code, count] : currentCounts) {
        allCounts[code] = count;
    }

    for (const auto& [code, ignoredCount] : allCounts) {
        static_cast<void>(ignoredCount);
        const int baselineCount = baselineCounts.count(code) > 0 ? baselineCounts[code] : 0;
        const int resolvedCurrentCount = currentCounts.count(code) > 0 ? currentCounts.at(code) : 0;
        if (resolvedCurrentCount > baselineCount) {
            delta.regressions.push_back({
                code,
                baselineCount,
                resolvedCurrentCount,
                resolvedCurrentCount - baselineCount
            });
        } else if (resolvedCurrentCount < baselineCount) {
            delta.improvements.push_back({
                code,
                baselineCount,
                resolvedCurrentCount,
                resolvedCurrentCount - baselineCount
            });
        }
    }

    std::map<std::string, int> baselineIssueCounts;
    std::map<std::string, ValidationBaselineIssueEntry> baselineIssueSamples;
    for (const auto& entry : snapshot.issueEntries) {
        const std::string signature = BuildIssueSignature(entry);
        ++baselineIssueCounts[signature];
        baselineIssueSamples[signature] = entry;
    }

    std::map<std::string, int> currentIssueCounts;
    std::map<std::string, ValidationBaselineIssueEntry> currentIssueSamples;
    for (const auto& issue : issues) {
        const ValidationBaselineIssueEntry entry = MakeBaselineIssueEntry(issue);
        const std::string signature = BuildIssueSignature(entry);
        ++currentIssueCounts[signature];
        currentIssueSamples[signature] = entry;
    }

    std::map<std::string, int> allIssueCounts = baselineIssueCounts;
    for (const auto& [signature, count] : currentIssueCounts) {
        allIssueCounts[signature] = count;
    }

    for (const auto& [signature, ignoredCount] : allIssueCounts) {
        static_cast<void>(ignoredCount);
        const int baselineCount = baselineIssueCounts.count(signature) > 0 ? baselineIssueCounts[signature] : 0;
        const int currentCount = currentIssueCounts.count(signature) > 0 ? currentIssueCounts[signature] : 0;
        if (currentCount > baselineCount) {
            const auto& sample = currentIssueSamples.at(signature);
            delta.issueRegressions.push_back({
                sample.severity,
                sample.code,
                sample.objectId,
                sample.scriptTag,
                sample.relatedValue,
                currentCount - baselineCount
            });
        } else if (currentCount < baselineCount) {
            const auto& sample = baselineIssueSamples.at(signature);
            delta.issueImprovements.push_back({
                sample.severity,
                sample.code,
                sample.objectId,
                sample.scriptTag,
                sample.relatedValue,
                baselineCount - currentCount
            });
        }
    }

    return delta;
}

void AppendExportAuditEntry(const WorldExportResult& result) {
    if (result.auditTrailPath.empty()) {
        return;
    }

    std::ofstream auditFile(result.auditTrailPath, std::ios::app);
    if (!auditFile.is_open()) {
        return;
    }

    auditFile << "=== " << result.generatedAt << " ===\n";
    auditFile << "Target: " << result.worldPath.string() << '\n';
    auditFile << "Policy: " << ExportValidationPolicyLabel(result.policy) << '\n';
    auditFile << "Decision: " << WorldExportDecisionLabel(result) << '\n';
    auditFile << "Errors: " << result.errorCount << '\n';
    auditFile << "Warnings: " << result.warningCount << '\n';
    auditFile << "Auto-created semantic anchors: " << result.autoCreatedSemanticAnchorCount << '\n';
    auditFile << "Report: " << result.validationReportPath.string() << '\n';
    auditFile << "Snapshot: " << result.validationSnapshotPath.string() << '\n';
    auditFile << "Baseline: " << result.baselineSnapshotPath.string() << '\n';
    auditFile << "Baseline updated: " << (result.baselineUpdated ? "yes" : "no") << '\n';
    auditFile << "Message: " << result.message << "\n\n";
}

}  // namespace

const char* ExportValidationPolicyLabel(ExportValidationPolicy policy) {
    switch (policy) {
    case ExportValidationPolicy::BlockAutoCreatedSemanticAnchors:
        return "shipping-safe";
    case ExportValidationPolicy::AllowWarnings:
    default:
        return "prototype / allow warnings";
    }
}

const char* WorldExportDecisionLabel(const WorldExportResult& result) {
    if (result.ok) {
        return "exported";
    }
    if (result.blockedByValidation) {
        return "blocked by validation";
    }
    if (result.blockedByPolicy) {
        return "blocked by policy";
    }
    if (result.saveFailed) {
        return "save failed";
    }
    return "not exported";
}

const char* WorldExportHistoryFilterLabel(WorldExportHistoryFilter filter) {
    switch (filter) {
    case WorldExportHistoryFilter::Successful:
        return "Successful";
    case WorldExportHistoryFilter::Blocked:
        return "Blocked";
    case WorldExportHistoryFilter::SaveFailed:
        return "Save Failed";
    case WorldExportHistoryFilter::PrototypeOnly:
        return "Prototype Only";
    case WorldExportHistoryFilter::ShippingOnly:
        return "Shipping Only";
    case WorldExportHistoryFilter::BaselineUpdatedOnly:
        return "Baseline Updated";
    case WorldExportHistoryFilter::All:
    default:
        return "All";
    }
}

const char* WorldExportComparePresetLabel(WorldExportComparePreset preset) {
    switch (preset) {
    case WorldExportComparePreset::LatestSuccessfulShipping:
        return "Latest Successful Shipping";
    case WorldExportComparePreset::LatestSuccessfulPrototype:
        return "Latest Successful Prototype";
    case WorldExportComparePreset::LatestBlocked:
        return "Latest Blocked";
    case WorldExportComparePreset::LatestBaselineUpdated:
        return "Latest Baseline Updated";
    case WorldExportComparePreset::ManualSelection:
    default:
        return "Manual Selection";
    }
}

namespace {

bool IsHistoryEntrySuccessful(const WorldExportHistoryEntry& entry) {
    return entry.decisionLabel == "exported";
}

bool IsHistoryEntrySaveFailed(const WorldExportHistoryEntry& entry) {
    return entry.decisionLabel == "save failed";
}

bool IsHistoryEntryBlocked(const WorldExportHistoryEntry& entry) {
    return !IsHistoryEntrySuccessful(entry) && !IsHistoryEntrySaveFailed(entry);
}

bool IsHistoryEntryPrototype(const WorldExportHistoryEntry& entry) {
    return entry.policyLabel == ExportValidationPolicyLabel(ExportValidationPolicy::AllowWarnings);
}

bool IsHistoryEntryShipping(const WorldExportHistoryEntry& entry) {
    return entry.policyLabel == ExportValidationPolicyLabel(ExportValidationPolicy::BlockAutoCreatedSemanticAnchors);
}

std::string DescribeHistoryQuery(const WorldExportHistoryQuery& query) {
    switch (query.filter) {
    case WorldExportHistoryFilter::Successful:
        return "successful export";
    case WorldExportHistoryFilter::Blocked:
        return "blocked export";
    case WorldExportHistoryFilter::SaveFailed:
        return "save-failed export";
    case WorldExportHistoryFilter::PrototypeOnly:
        return query.requireSuccessful ? "successful prototype export" : "prototype export";
    case WorldExportHistoryFilter::ShippingOnly:
        return query.requireSuccessful ? "successful shipping export" : "shipping export";
    case WorldExportHistoryFilter::BaselineUpdatedOnly:
        return "baseline-updating export";
    case WorldExportHistoryFilter::All:
    default:
        return query.requireSuccessful ? "successful export" : "audit checkpoint";
    }
}

WorldExportHistorySelection MakeHistorySelection(const std::vector<WorldExportHistoryEntry>& entries, int historyIndex) {
    WorldExportHistorySelection selection;
    if (historyIndex < 0 || historyIndex >= static_cast<int>(entries.size())) {
        return selection;
    }

    const auto& entry = entries[static_cast<std::size_t>(historyIndex)];
    selection.found = true;
    selection.historyIndex = historyIndex;
    selection.summaryLabel = SummarizeWorldExportHistoryEntry(entry);
    selection.badgeLabel = BuildWorldExportHistoryBadgeLabel(entry);
    return selection;
}

}  // namespace

std::filesystem::path ValidationReportPathForWorld(const std::filesystem::path& worldPath) {
    const std::string reportFileName = worldPath.stem().string() + ".validation.txt";
    return worldPath.parent_path() / reportFileName;
}

std::filesystem::path ExportAuditTrailPathForWorld(const std::filesystem::path& worldPath) {
    const std::string auditFileName = worldPath.stem().string() + ".export-history.txt";
    return worldPath.parent_path() / auditFileName;
}

std::filesystem::path ValidationSnapshotArchivePathForWorld(const std::filesystem::path& worldPath, std::string_view exportToken) {
    const std::string snapshotFileName = worldPath.stem().string() + ".validation-snapshot-" + std::string(exportToken) + ".txt";
    return worldPath.parent_path() / snapshotFileName;
}

std::filesystem::path ValidationBaselinePathForWorld(const std::filesystem::path& worldPath) {
    const std::string baselineFileName = worldPath.stem().string() + ".shipping-baseline.txt";
    return worldPath.parent_path() / baselineFileName;
}

std::string LoadTextArtifactPreview(const std::filesystem::path& path, std::size_t maxChars) {
    if (path.empty()) {
        return "No artifact target selected.";
    }

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return "Artifact not found: " + path.string();
    }

    const std::string content(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
    if (content.empty()) {
        return "Artifact is empty: " + path.string();
    }
    if (content.size() <= maxChars) {
        return content;
    }

    return "[showing last " + std::to_string(maxChars) + " chars of " + std::to_string(content.size()) + "]\n" +
        content.substr(content.size() - maxChars);
}

bool LoadWorldExportHistory(const std::filesystem::path& worldPath, std::vector<WorldExportHistoryEntry>& entries) {
    entries.clear();

    std::ifstream file(ExportAuditTrailPathForWorld(worldPath));
    if (!file.is_open()) {
        return false;
    }

    WorldExportHistoryEntry currentEntry;
    bool inEntry = false;
    std::string line;
    while (std::getline(file, line)) {
        if (line.rfind("=== ", 0) == 0 && line.size() > 8 && line.substr(line.size() - 4) == " ===") {
            if (inEntry) {
                entries.push_back(currentEntry);
            }
            currentEntry = {};
            currentEntry.generatedAt = line.substr(4, line.size() - 8);
            inEntry = true;
            continue;
        }
        if (!inEntry) {
            continue;
        }

        std::string parsedValue = TrimLinePrefix(line, "Target: ");
        if (!parsedValue.empty()) {
            currentEntry.targetPath = parsedValue;
            continue;
        }
        parsedValue = TrimLinePrefix(line, "Policy: ");
        if (!parsedValue.empty()) {
            currentEntry.policyLabel = parsedValue;
            continue;
        }
        parsedValue = TrimLinePrefix(line, "Decision: ");
        if (!parsedValue.empty()) {
            currentEntry.decisionLabel = parsedValue;
            continue;
        }
        parsedValue = TrimLinePrefix(line, "Errors: ");
        if (!parsedValue.empty()) {
            TryParseInt(parsedValue, currentEntry.errorCount);
            continue;
        }
        parsedValue = TrimLinePrefix(line, "Warnings: ");
        if (!parsedValue.empty()) {
            TryParseInt(parsedValue, currentEntry.warningCount);
            continue;
        }
        parsedValue = TrimLinePrefix(line, "Auto-created semantic anchors: ");
        if (!parsedValue.empty()) {
            TryParseInt(parsedValue, currentEntry.autoCreatedSemanticAnchorCount);
            continue;
        }
        parsedValue = TrimLinePrefix(line, "Report: ");
        if (!parsedValue.empty()) {
            currentEntry.validationReportPath = parsedValue;
            continue;
        }
        parsedValue = TrimLinePrefix(line, "Snapshot: ");
        if (!parsedValue.empty()) {
            currentEntry.validationSnapshotPath = parsedValue;
            continue;
        }
        parsedValue = TrimLinePrefix(line, "Baseline: ");
        if (!parsedValue.empty()) {
            currentEntry.baselineSnapshotPath = parsedValue;
            continue;
        }
        parsedValue = TrimLinePrefix(line, "Baseline updated: ");
        if (!parsedValue.empty()) {
            currentEntry.baselineUpdated = ParseYesNo(parsedValue);
            continue;
        }
        parsedValue = TrimLinePrefix(line, "Message: ");
        if (!parsedValue.empty()) {
            currentEntry.message = parsedValue;
        }
    }

    if (inEntry) {
        entries.push_back(currentEntry);
    }

    std::reverse(entries.begin(), entries.end());
    return true;
}

bool MatchesHistoryQuery(const WorldExportHistoryEntry& entry, const WorldExportHistoryQuery& query) {
    bool matchesFilter = false;
    switch (query.filter) {
    case WorldExportHistoryFilter::Successful:
        matchesFilter = IsHistoryEntrySuccessful(entry);
        break;
    case WorldExportHistoryFilter::Blocked:
        matchesFilter = IsHistoryEntryBlocked(entry);
        break;
    case WorldExportHistoryFilter::SaveFailed:
        matchesFilter = IsHistoryEntrySaveFailed(entry);
        break;
    case WorldExportHistoryFilter::PrototypeOnly:
        matchesFilter = IsHistoryEntryPrototype(entry);
        break;
    case WorldExportHistoryFilter::ShippingOnly:
        matchesFilter = IsHistoryEntryShipping(entry);
        break;
    case WorldExportHistoryFilter::BaselineUpdatedOnly:
        matchesFilter = entry.baselineUpdated;
        break;
    case WorldExportHistoryFilter::All:
    default:
        matchesFilter = true;
        break;
    }

    if (!matchesFilter) {
        return false;
    }
    if (query.requireSuccessful && !IsHistoryEntrySuccessful(entry)) {
        return false;
    }
    if (query.requireValidationSnapshot && entry.validationSnapshotPath.empty()) {
        return false;
    }
    return true;
}

std::vector<WorldExportHistorySelection> FilterWorldExportHistoryEntries(
    const std::vector<WorldExportHistoryEntry>& entries,
    const WorldExportHistoryQuery& query) {
    std::vector<WorldExportHistorySelection> filteredEntries;
    filteredEntries.reserve(entries.size());

    for (int historyIndex = 0; historyIndex < static_cast<int>(entries.size()); ++historyIndex) {
        const auto& entry = entries[static_cast<std::size_t>(historyIndex)];
        if (!MatchesHistoryQuery(entry, query)) {
            continue;
        }

        WorldExportHistorySelection selection = MakeHistorySelection(entries, historyIndex);
        selection.query = query;
        filteredEntries.push_back(std::move(selection));
    }

    return filteredEntries;
}

WorldExportHistorySelection FindLatestMatchingHistoryEntry(
    const std::vector<WorldExportHistoryEntry>& entries,
    const WorldExportHistoryQuery& query) {
    for (int historyIndex = 0; historyIndex < static_cast<int>(entries.size()); ++historyIndex) {
        const auto& entry = entries[static_cast<std::size_t>(historyIndex)];
        if (!MatchesHistoryQuery(entry, query)) {
            continue;
        }

        WorldExportHistorySelection selection = MakeHistorySelection(entries, historyIndex);
        selection.query = query;
        return selection;
    }

    WorldExportHistorySelection selection;
    selection.query = query;
    selection.fallbackMessage = "No " + DescribeHistoryQuery(query) +
        (query.requireValidationSnapshot
            ? " with an archived validation snapshot was found."
            : " was found.");
    return selection;
}

WorldExportHistorySelection ResolveComparePresetTarget(
    const std::vector<WorldExportHistoryEntry>& entries,
    WorldExportComparePreset preset,
    int manualSelectionIndex) {
    if (preset == WorldExportComparePreset::ManualSelection) {
        WorldExportHistorySelection selection = MakeHistorySelection(entries, manualSelectionIndex);
        selection.preset = preset;
        selection.query.requireValidationSnapshot = true;
        if (!selection.found) {
            selection.fallbackMessage = "Select a historical export checkpoint to compare against.";
            return selection;
        }

        const auto& entry = entries[static_cast<std::size_t>(manualSelectionIndex)];
        if (entry.validationSnapshotPath.empty()) {
            selection.found = false;
            selection.fallbackMessage =
                "Selected historical export checkpoint has no archived validation snapshot.";
        }
        return selection;
    }

    WorldExportHistoryQuery query;
    query.requireValidationSnapshot = true;
    switch (preset) {
    case WorldExportComparePreset::LatestSuccessfulShipping:
        query.filter = WorldExportHistoryFilter::ShippingOnly;
        query.requireSuccessful = true;
        break;
    case WorldExportComparePreset::LatestSuccessfulPrototype:
        query.filter = WorldExportHistoryFilter::PrototypeOnly;
        query.requireSuccessful = true;
        break;
    case WorldExportComparePreset::LatestBlocked:
        query.filter = WorldExportHistoryFilter::Blocked;
        break;
    case WorldExportComparePreset::LatestBaselineUpdated:
        query.filter = WorldExportHistoryFilter::BaselineUpdatedOnly;
        break;
    case WorldExportComparePreset::ManualSelection:
    default:
        break;
    }

    WorldExportHistorySelection selection = FindLatestMatchingHistoryEntry(entries, query);
    selection.preset = preset;
    return selection;
}

std::string BuildWorldExportHistoryBadgeLabel(const WorldExportHistoryEntry& entry) {
    std::vector<std::string> badges;
    if (IsHistoryEntryShipping(entry)) {
        badges.emplace_back("shipping");
    } else if (IsHistoryEntryPrototype(entry)) {
        badges.emplace_back("prototype");
    } else if (!entry.policyLabel.empty()) {
        badges.push_back(entry.policyLabel);
    }

    if (IsHistoryEntrySuccessful(entry)) {
        badges.emplace_back("ok");
    } else if (IsHistoryEntrySaveFailed(entry)) {
        badges.emplace_back("save-failed");
    } else if (IsHistoryEntryBlocked(entry)) {
        badges.emplace_back("blocked");
    } else if (!entry.decisionLabel.empty()) {
        badges.push_back(entry.decisionLabel);
    }

    if (entry.baselineUpdated) {
        badges.emplace_back("baseline");
    }

    std::ostringstream label;
    for (std::size_t badgeIndex = 0; badgeIndex < badges.size(); ++badgeIndex) {
        if (badgeIndex > 0) {
            label << ' ';
        }
        label << '[' << badges[badgeIndex] << ']';
    }
    return label.str();
}

std::string SummarizeWorldExportHistoryEntry(const WorldExportHistoryEntry& entry) {
    std::ostringstream summary;
    summary << entry.generatedAt;
    const std::string badgeLabel = BuildWorldExportHistoryBadgeLabel(entry);
    if (!badgeLabel.empty()) {
        summary << " | " << badgeLabel;
    }
    summary << " | " << entry.errorCount << "E/" << entry.warningCount << "W";
    if (entry.autoCreatedSemanticAnchorCount > 0) {
        summary << " | auto " << entry.autoCreatedSemanticAnchorCount;
    }
    return summary.str();
}

std::string BuildValidationBaselineSnapshot(const std::vector<ValidationIssue>& issues, const WorldExportResult& result) {
    std::ostringstream snapshot;
    snapshot << "World Validation Snapshot\n";
    snapshot << "Generated: " << result.generatedAt << '\n';
    snapshot << "Target: " << result.worldPath.string() << '\n';
    snapshot << "Policy: " << ExportValidationPolicyLabel(result.policy) << '\n';
    snapshot << "Decision: " << WorldExportDecisionLabel(result) << '\n';
    snapshot << "Errors: " << result.errorCount << '\n';
    snapshot << "Warnings: " << result.warningCount << '\n';
    snapshot << "Auto-created semantic anchors: " << result.autoCreatedSemanticAnchorCount << '\n';
    snapshot << "IssueCounts:\n";
    for (const auto& tally : BuildIssueTallies(issues)) {
        snapshot << "- " << tally.code << " = " << tally.count << '\n';
    }
    snapshot << "IssueEntries:\n";
    for (const auto& entry : BuildBaselineIssueEntries(issues)) {
        snapshot << "- "
                 << BaselineSeverityLabel(entry.severity) << '\t'
                 << entry.code << '\t'
                 << entry.objectId << '\t'
                 << entry.scriptTag << '\t'
                 << entry.relatedValue << '\n';
    }
    return snapshot.str();
}

bool LoadValidationBaselineSnapshot(const std::filesystem::path& path, ValidationBaselineSnapshot& snapshot) {
    snapshot = {};
    snapshot.path = path;

    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    bool readingIssueCounts = false;
    bool readingIssueEntries = false;
    std::string line;
    while (std::getline(file, line)) {
        if (line == "IssueCounts:") {
            readingIssueCounts = true;
            readingIssueEntries = false;
            continue;
        }
        if (line == "IssueEntries:") {
            readingIssueCounts = false;
            readingIssueEntries = true;
            continue;
        }

        if (readingIssueCounts) {
            if (line.rfind("- ", 0) != 0) {
                continue;
            }

            const std::string entry = line.substr(2);
            const std::size_t separator = entry.find(" = ");
            if (separator == std::string::npos) {
                continue;
            }

            ValidationIssueTally tally;
            tally.code = entry.substr(0, separator);
            if (!TryParseInt(entry.substr(separator + 3), tally.count)) {
                continue;
            }
            snapshot.issueCounts.push_back(tally);
            continue;
        }
        if (readingIssueEntries) {
            if (line.rfind("- ", 0) != 0) {
                continue;
            }

            const auto parts = SplitTabSeparated(line.substr(2));
            if (parts.size() < 5) {
                continue;
            }

            ValidationBaselineIssueEntry entry;
            entry.severity = ParseBaselineSeverityLabel(parts[0]);
            entry.code = parts[1];
            entry.objectId = parts[2];
            entry.scriptTag = parts[3];
            entry.relatedValue = parts[4];
            snapshot.issueEntries.push_back(std::move(entry));
            continue;
        }

        std::string parsedValue = TrimLinePrefix(line, "Generated: ");
        if (!parsedValue.empty()) {
            snapshot.generatedAt = parsedValue;
            continue;
        }
        parsedValue = TrimLinePrefix(line, "Target: ");
        if (!parsedValue.empty()) {
            snapshot.targetPath = parsedValue;
            continue;
        }
        parsedValue = TrimLinePrefix(line, "Policy: ");
        if (!parsedValue.empty()) {
            snapshot.policyLabel = parsedValue;
            continue;
        }
        parsedValue = TrimLinePrefix(line, "Decision: ");
        if (!parsedValue.empty()) {
            snapshot.decisionLabel = parsedValue;
            continue;
        }
        parsedValue = TrimLinePrefix(line, "Errors: ");
        if (!parsedValue.empty()) {
            TryParseInt(parsedValue, snapshot.errorCount);
            continue;
        }
        parsedValue = TrimLinePrefix(line, "Warnings: ");
        if (!parsedValue.empty()) {
            TryParseInt(parsedValue, snapshot.warningCount);
            continue;
        }
        parsedValue = TrimLinePrefix(line, "Auto-created semantic anchors: ");
        if (!parsedValue.empty()) {
            TryParseInt(parsedValue, snapshot.autoCreatedSemanticAnchorCount);
        }
    }

    snapshot.loaded = true;
    return true;
}

ValidationBaselineDelta CompareValidationToSnapshot(const std::vector<ValidationIssue>& issues, const std::filesystem::path& snapshotPath) {
    ValidationBaselineDelta delta;
    delta.baselinePath = snapshotPath;

    ValidationBaselineSnapshot snapshot;
    if (!LoadValidationBaselineSnapshot(snapshotPath, snapshot)) {
        return delta;
    }

    return CompareValidationToLoadedSnapshot(issues, snapshot, snapshotPath);
}

ValidationBaselineDelta CompareValidationToBaseline(const std::vector<ValidationIssue>& issues, const std::filesystem::path& worldPath) {
    return CompareValidationToSnapshot(issues, ValidationBaselinePathForWorld(worldPath));
}

std::string BuildValidationSnapshotDeltaReport(const ValidationBaselineDelta& delta, std::string_view label) {
    const std::string labelText = std::string(label);
    if (!delta.hasBaseline) {
        return "No validation snapshot found for " + labelText + ".";
    }

    std::ostringstream report;
    report << labelText << " diff\n";
    report << "Baseline: " << delta.baselinePath.string() << '\n';
    report << "Errors: current " << delta.currentErrorCount << " vs baseline " << delta.baselineErrorCount << '\n';
    report << "Warnings: current " << delta.currentWarningCount << " vs baseline " << delta.baselineWarningCount << '\n';
    report << "Auto-created semantic anchors: current " << delta.currentAutoCreatedSemanticAnchorCount
           << " vs baseline " << delta.baselineAutoCreatedSemanticAnchorCount << '\n';

    if (delta.regressions.empty() && delta.improvements.empty()) {
        report << "No issue-count drift against " << labelText << ".\n";
    } else {
        if (!delta.regressions.empty()) {
            report << "Regressions:\n";
            for (const auto& regression : delta.regressions) {
                report << "- " << regression.code << ": " << regression.baselineCount
                       << " -> " << regression.currentCount << " (+" << regression.delta << ")\n";
            }
        } else {
            report << "Regressions: none\n";
        }

        if (!delta.improvements.empty()) {
            report << "Improvements:\n";
            for (const auto& improvement : delta.improvements) {
                report << "- " << improvement.code << ": " << improvement.baselineCount
                       << " -> " << improvement.currentCount << " (" << improvement.delta << ")\n";
            }
        } else {
            report << "Improvements: none\n";
        }
    }

    if (!delta.issueRegressions.empty()) {
        report << "Detailed regressions:\n";
        for (const auto& regression : delta.issueRegressions) {
            report << "- " << FormatBaselineIssueDeltaEntry(regression) << '\n';
        }
    } else {
        report << "Detailed regressions: none\n";
    }

    if (!delta.issueImprovements.empty()) {
        report << "Detailed improvements:\n";
        for (const auto& improvement : delta.issueImprovements) {
            report << "- " << FormatBaselineIssueDeltaEntry(improvement) << '\n';
        }
    } else {
        report << "Detailed improvements: none\n";
    }

    return report.str();
}

std::string BuildValidationBaselineDeltaReport(const ValidationBaselineDelta& delta) {
    return BuildValidationSnapshotDeltaReport(delta, "shipping baseline");
}

std::string BuildWorldValidationReport(const std::vector<ValidationIssue>& issues, const WorldExportResult& result) {
    std::ostringstream report;
    report << "World Validation Report\n";
    report << "Generated: " << result.generatedAt << '\n';
    report << "Target: " << result.worldPath.string() << '\n';
    report << "Policy: " << ExportValidationPolicyLabel(result.policy) << '\n';
    report << "Decision: " << WorldExportDecisionLabel(result) << '\n';
    report << "Summary: " << BuildValidationSummary(issues) << '\n';
    report << "Errors: " << result.errorCount << '\n';
    report << "Warnings: " << result.warningCount << '\n';
    report << "Auto-created semantic anchors: " << result.autoCreatedSemanticAnchorCount << '\n';
    report << "Audit trail: " << result.auditTrailPath.string() << '\n';
    report << "Validation snapshot: " << result.validationSnapshotPath.string() << '\n';
    report << "Shipping baseline: " << result.baselineSnapshotPath.string() << '\n';
    report << "Shipping baseline updated: " << (result.baselineUpdated ? "yes" : "no") << '\n';
    report << "Message: " << result.message << '\n';

    if (issues.empty()) {
        report << "Issues: none\n";
        return report.str();
    }

    report << "Issues:\n";
    for (const auto& issue : issues) {
        report << "- [" << (issue.severity == ValidationSeverity::Error ? "Error" : "Warning") << "] "
               << issue.code;
        if (!issue.objectId.empty()) {
            report << " :: " << issue.objectId;
        }
        if (!issue.scriptTag.empty()) {
            report << " :: scriptTag=" << issue.scriptTag;
        }
        if (!issue.relatedValue.empty()) {
            report << " :: related=" << issue.relatedValue;
        }
        report << '\n';
        report << "  " << issue.message << '\n';
    }

    return report.str();
}

WorldExportResult ExportWorldWithValidation(const World& world,
    const std::filesystem::path& path,
    ExportValidationPolicy policy) {
    WorldExportResult result;
    result.policy = policy;
    result.worldPath = path;
    result.validationReportPath = ValidationReportPathForWorld(path);
    result.auditTrailPath = ExportAuditTrailPathForWorld(path);
    result.validationSnapshotPath = ValidationSnapshotArchivePathForWorld(path, CurrentLocalTimestampToken());
    result.baselineSnapshotPath = ValidationBaselinePathForWorld(path);
    result.generatedAt = CurrentLocalTimestamp();

    const auto issues = ValidateWorldForRuntime(world);
    result.errorCount = CountValidationErrors(issues);
    result.warningCount = CountValidationWarnings(issues);
    result.autoCreatedSemanticAnchorCount = CountValidationIssuesByCode(issues, "auto_created_semantic_anchor");

    if (HasBlockingValidationIssues(issues)) {
        result.blockedByValidation = true;
        result.message = "Export blocked: " + BuildValidationSummary(issues) +
            " Report: " + result.validationReportPath.string() +
            " Audit: " + result.auditTrailPath.string();
    } else {
        std::string policyReason;
        if (ExportPolicyBlocksWorld(issues, policy, policyReason)) {
            result.blockedByPolicy = true;
            result.message = "Export blocked by policy: " + policyReason +
                ". Report: " + result.validationReportPath.string() +
                " Audit: " + result.auditTrailPath.string();
        } else {
            const auto saveResult = SaveWorldAtomically(world, path);
            if (!saveResult.ok) {
                result.saveFailed = true;
                result.message = "Export failed: " + saveResult.message +
                    " Audit: " + result.auditTrailPath.string();
            } else {
                result.ok = true;
                if (result.warningCount > 0) {
                    result.message = "Exported with warnings (" + std::to_string(result.warningCount) +
                        ", auto semantic anchors: " + std::to_string(result.autoCreatedSemanticAnchorCount) +
                        "). Report: " + result.validationReportPath.string() +
                        " Audit: " + result.auditTrailPath.string();
                } else {
                    result.message = "Exported world. Report: " + result.validationReportPath.string() +
                        " Audit: " + result.auditTrailPath.string();
                }
            }
        }
    }

    {
        std::ofstream snapshotFile(result.validationSnapshotPath);
        if (snapshotFile.is_open()) {
            snapshotFile << BuildValidationBaselineSnapshot(issues, result);
        }
    }

    if (result.ok && result.policy == ExportValidationPolicy::BlockAutoCreatedSemanticAnchors) {
        std::ofstream baselineFile(result.baselineSnapshotPath);
        if (baselineFile.is_open()) {
            baselineFile << BuildValidationBaselineSnapshot(issues, result);
            result.baselineUpdated = true;
            result.message += " Snapshot: " + result.validationSnapshotPath.string();
            result.message += " Baseline: " + result.baselineSnapshotPath.string();
        }
    }

    {
        std::ofstream reportFile(result.validationReportPath);
        if (reportFile.is_open()) {
            reportFile << BuildWorldValidationReport(issues, result);
        }
    }
    AppendExportAuditEntry(result);
    return result;
}

}  // namespace bunker
