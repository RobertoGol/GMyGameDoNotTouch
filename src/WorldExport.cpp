#include "../include/WorldExport.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>

#include "../include/AtomicPersistence.hpp"
#include "../include/PrefabLibrary.hpp"

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

std::string NormalizeFileExtension(std::string_view extension) {
    std::string normalized(extension);
    if (normalized.empty()) {
        return {};
    }
    if (normalized.front() != '.') {
        normalized.insert(normalized.begin(), '.');
    }
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char value) {
        return static_cast<char>(std::tolower(value));
    });
    return normalized;
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

const char* SupportedFileFormatLayerLabel(SupportedFileFormatLayer layer) {
    switch (layer) {
    case SupportedFileFormatLayer::BunkerWorld:
        return "Bunker Protocol world/record";
    case SupportedFileFormatLayer::BunkerPackage:
        return "Bunker package/archive";
    case SupportedFileFormatLayer::RecordPlugin:
        return "Creation-style record/plugin";
    case SupportedFileFormatLayer::AssetArchive:
        return "Asset archive";
    case SupportedFileFormatLayer::RuntimeSave:
        return "Runtime save/sidecar";
    case SupportedFileFormatLayer::MeshModelGeometry:
        return "Mesh/model/geometry";
    case SupportedFileFormatLayer::AnimationPhysics:
        return "Animation/physics/behavior";
    case SupportedFileFormatLayer::Texture:
        return "Texture/source image";
    case SupportedFileFormatLayer::MaterialShader:
        return "Material/shader";
    case SupportedFileFormatLayer::Script:
        return "Script source/compiled";
    case SupportedFileFormatLayer::AudioVoiceLip:
        return "Audio/voice/lip";
    case SupportedFileFormatLayer::Localization:
        return "Localization strings";
    case SupportedFileFormatLayer::InterfaceUI:
        return "Interface/UI";
    case SupportedFileFormatLayer::GeneratedWorldData:
        return "Generated world/dialogue support";
    case SupportedFileFormatLayer::ConfigTextLog:
        return "Config/text/log/meta";
    case SupportedFileFormatLayer::ModPackage:
        return "Mod/package/tooling";
    case SupportedFileFormatLayer::ExecutableNativePlugin:
        return "Executable/native plugin";
    default:
        return "Config/text/log/meta";
    }
}

const std::vector<SupportedFileFormat>& SupportedFileFormats() {
    static const std::vector<SupportedFileFormat> formats = {
        {".bwld", SupportedFileFormatLayer::BunkerWorld, "Bunker Protocol world", "Project-native source-of-truth world/record file.", true, true, false, false, false, true, true},
        {".dba", SupportedFileFormatLayer::BunkerPackage, "Data Bunker Archive", "Bunker-native DLC/mod package reference.", false, true, true, true, false, false, true},
        {".bmanifest", SupportedFileFormatLayer::BunkerPackage, "Bunker manifest", "Bunker package metadata reference.", false, true, true, true, false, true, true},
        {".bcluster", SupportedFileFormatLayer::BunkerPackage, "Bunker cluster manifest", "Planned Bunker world-cluster metadata reference.", false, true, false, true, false, true, true},
        {".esm", SupportedFileFormatLayer::RecordPlugin, "Creation master plugin", "Reference class only; not imported/exported as Fallout data.", false, false, false, true, false, false, true},
        {".esp", SupportedFileFormatLayer::RecordPlugin, "Creation plugin", "Reference class only; not imported/exported as Fallout data.", false, false, false, true, false, false, true},
        {".esl", SupportedFileFormatLayer::RecordPlugin, "Creation light plugin", "Reference class only; not imported/exported as Fallout data.", false, false, false, true, false, false, true},
        {".bsa", SupportedFileFormatLayer::AssetArchive, "Bethesda asset archive", "External asset archive reference only.", false, false, false, true, false, false, true},
        {".ba2", SupportedFileFormatLayer::AssetArchive, "Creation asset archive", "External asset archive reference only.", false, false, false, true, false, false, true},
        {".fos", SupportedFileFormatLayer::RuntimeSave, "Fallout save", "Runtime save reference class; not an authoring world file.", false, false, false, true, false, false, false},
        {".f4se", SupportedFileFormatLayer::RuntimeSave, "Script extender co-save", "Runtime sidecar reference class.", false, false, false, true, false, false, false},
        {".nvse", SupportedFileFormatLayer::RuntimeSave, "NVSE sidecar", "Runtime sidecar reference class.", false, false, false, true, false, false, false},
        {".obse", SupportedFileFormatLayer::RuntimeSave, "OBSE sidecar", "Runtime sidecar reference class.", false, false, false, true, false, false, false},
        {".skse", SupportedFileFormatLayer::RuntimeSave, "SKSE sidecar", "Runtime sidecar reference class.", false, false, false, true, false, false, false},
        {".nif", SupportedFileFormatLayer::MeshModelGeometry, "NetImmerse/Gamebryo mesh", "Mesh/model asset reference class.", false, false, false, true, false, false, true},
        {".tri", SupportedFileFormatLayer::MeshModelGeometry, "Morph/face geometry", "Face or morph geometry reference class.", false, false, false, true, false, false, true},
        {".egm", SupportedFileFormatLayer::MeshModelGeometry, "FaceGen morph", "FaceGen morph reference class.", false, false, false, true, false, false, true},
        {".egt", SupportedFileFormatLayer::MeshModelGeometry, "FaceGen tint/morph", "FaceGen tint/morph reference class.", false, false, false, true, false, false, true},
        {".ctl", SupportedFileFormatLayer::MeshModelGeometry, "FaceGen control", "FaceGen control reference class.", false, false, false, true, false, false, true},
        {".rdt", SupportedFileFormatLayer::MeshModelGeometry, "Legacy resource data", "Legacy geometry/resource reference class.", false, false, false, true, false, false, true},
        {".spt", SupportedFileFormatLayer::MeshModelGeometry, "SpeedTree asset", "Vegetation/SpeedTree reference class.", false, false, false, true, false, false, true},
        {".navmesh", SupportedFileFormatLayer::GeneratedWorldData, "Navmesh reference", "Loose navmesh-style reference marker.", false, false, false, true, false, false, true},
        {".kf", SupportedFileFormatLayer::AnimationPhysics, "Animation clip", "Legacy animation reference class.", false, false, false, true, false, false, true},
        {".kfm", SupportedFileFormatLayer::AnimationPhysics, "Animation controller", "Animation controller reference class.", false, false, false, true, false, false, true},
        {".hkx", SupportedFileFormatLayer::AnimationPhysics, "Havok behavior/animation", "Animation/behavior/physics reference class.", false, false, false, true, false, false, true},
        {".dds", SupportedFileFormatLayer::Texture, "DirectDraw texture", "Runtime texture asset reference class.", false, false, false, true, false, false, true},
        {".tga", SupportedFileFormatLayer::Texture, "Targa source texture", "Source/legacy texture reference class.", false, false, false, true, false, true, true},
        {".png", SupportedFileFormatLayer::Texture, "PNG source texture", "Tooling/source texture reference class.", false, false, false, true, false, true, false},
        {".jpg", SupportedFileFormatLayer::Texture, "JPEG source texture", "Tooling/source texture reference class.", false, false, false, true, false, true, false},
        {".jpeg", SupportedFileFormatLayer::Texture, "JPEG source texture", "Tooling/source texture reference class.", false, false, false, true, false, true, false},
        {".bmp", SupportedFileFormatLayer::Texture, "Bitmap source texture", "Tooling/source texture reference class.", false, false, false, true, false, true, false},
        {".bgsm", SupportedFileFormatLayer::MaterialShader, "Material", "Material asset reference class.", false, false, false, true, false, false, true},
        {".bgem", SupportedFileFormatLayer::MaterialShader, "Effect material", "Effect material reference class.", false, false, false, true, false, false, true},
        {".mat", SupportedFileFormatLayer::MaterialShader, "Generic material", "Material reference class.", false, false, false, true, false, true, true},
        {".fx", SupportedFileFormatLayer::MaterialShader, "Shader/effect", "Shader/effect reference class.", false, false, false, true, false, true, true},
        {".cub", SupportedFileFormatLayer::MaterialShader, "Cubemap", "Environment cubemap reference class.", false, false, false, true, false, false, true},
        {".psc", SupportedFileFormatLayer::Script, "Papyrus source", "Script source reference class.", false, false, false, true, false, true, true},
        {".pex", SupportedFileFormatLayer::Script, "Papyrus compiled script", "Compiled script reference class.", false, false, false, true, false, false, true},
        {".dll", SupportedFileFormatLayer::ExecutableNativePlugin, "Native plugin", "Dangerous executable/native plugin reference. Never load automatically.", false, false, false, true, true, false, false},
        {".exe", SupportedFileFormatLayer::ExecutableNativePlugin, "Executable", "Dangerous executable reference. Never launch from scan results.", false, false, false, true, true, false, false},
        {".fuz", SupportedFileFormatLayer::AudioVoiceLip, "Voice package", "Audio/voice reference class.", false, false, false, true, false, false, true},
        {".lip", SupportedFileFormatLayer::AudioVoiceLip, "Lip sync", "Lip animation reference class.", false, false, false, true, false, false, true},
        {".xwm", SupportedFileFormatLayer::AudioVoiceLip, "XWMA audio", "Audio asset reference class.", false, false, false, true, false, false, true},
        {".wav", SupportedFileFormatLayer::AudioVoiceLip, "Wave audio", "Audio asset reference class.", false, false, false, true, false, false, true},
        {".ogg", SupportedFileFormatLayer::AudioVoiceLip, "Legacy audio", "Legacy audio/music reference class.", false, false, false, true, false, false, true},
        {".mp3", SupportedFileFormatLayer::AudioVoiceLip, "Music/audio source", "Audio source reference class.", false, false, false, true, false, false, false},
        {".dat", SupportedFileFormatLayer::AudioVoiceLip, "Legacy generated audio/lip data", "Generated audio/lip sidecar reference class.", false, false, false, true, false, false, true},
        {".strings", SupportedFileFormatLayer::Localization, "Localized strings", "Localization reference class.", false, false, false, true, false, false, true},
        {".dlstrings", SupportedFileFormatLayer::Localization, "Localized dialogue strings", "Localization reference class.", false, false, false, true, false, false, true},
        {".ilstrings", SupportedFileFormatLayer::Localization, "Localized interface strings", "Localization reference class.", false, false, false, true, false, false, true},
        {".swf", SupportedFileFormatLayer::InterfaceUI, "Scaleform UI asset", "Interface/UI reference class.", false, false, false, true, false, false, true},
        {".gfx", SupportedFileFormatLayer::InterfaceUI, "Scaleform GFx asset", "Interface/UI reference class.", false, false, false, true, false, false, true},
        {".fnt", SupportedFileFormatLayer::InterfaceUI, "Font asset", "Interface/font reference class.", false, false, false, true, false, false, true},
        {".seq", SupportedFileFormatLayer::GeneratedWorldData, "Quest sequence sidecar", "Generated/dialogue sidecar reference class.", false, false, false, true, false, false, true},
        {".lod", SupportedFileFormatLayer::GeneratedWorldData, "LOD reference", "Generic LOD/generated world-data reference class.", false, false, false, true, false, false, true},
        {".dlod", SupportedFileFormatLayer::GeneratedWorldData, "Distant LOD reference", "Distant LOD/generated world-data reference class.", false, false, false, true, false, false, true},
        {".bto", SupportedFileFormatLayer::GeneratedWorldData, "Object LOD", "Generated object LOD reference class.", false, false, false, true, false, false, true},
        {".btr", SupportedFileFormatLayer::GeneratedWorldData, "Terrain LOD", "Generated terrain LOD reference class.", false, false, false, true, false, false, true},
        {".btt", SupportedFileFormatLayer::GeneratedWorldData, "Tree LOD", "Generated tree LOD reference class.", false, false, false, true, false, false, true},
        {".previs", SupportedFileFormatLayer::GeneratedWorldData, "Previs reference", "Generated previsibility reference class.", false, false, false, true, false, false, true},
        {".precombined", SupportedFileFormatLayer::GeneratedWorldData, "Precombined reference", "Generated precombined geometry reference class.", false, false, false, true, false, false, true},
        {".ini", SupportedFileFormatLayer::ConfigTextLog, "Configuration", "Config reference class.", false, false, false, false, false, true, false},
        {".toml", SupportedFileFormatLayer::ConfigTextLog, "TOML configuration", "Config reference class.", false, false, false, false, false, true, false},
        {".yaml", SupportedFileFormatLayer::ConfigTextLog, "YAML configuration", "Config reference class.", false, false, false, false, false, true, false},
        {".yml", SupportedFileFormatLayer::ConfigTextLog, "YAML configuration", "Config reference class.", false, false, false, false, false, true, false},
        {".json", SupportedFileFormatLayer::ConfigTextLog, "JSON metadata", "Tooling metadata/config reference class.", false, false, false, false, false, true, false},
        {".xml", SupportedFileFormatLayer::ConfigTextLog, "XML metadata", "Tooling metadata/config reference class.", false, false, false, false, false, true, false},
        {".txt", SupportedFileFormatLayer::ConfigTextLog, "Text report", "Human-readable report/tooling artifact.", false, false, false, false, false, true, false},
        {".log", SupportedFileFormatLayer::ConfigTextLog, "Log", "Human-readable runtime/tooling log.", false, false, false, false, false, true, false},
        {".csv", SupportedFileFormatLayer::ConfigTextLog, "Delimited table", "Table/metadata source reference class.", false, false, false, false, false, true, false},
        {".tsv", SupportedFileFormatLayer::ConfigTextLog, "Delimited table", "Table/metadata source reference class.", false, false, false, false, false, true, false},
        {".zip", SupportedFileFormatLayer::ModPackage, "Archive package", "Compressed package reference only.", false, false, true, true, false, false, false},
        {".7z", SupportedFileFormatLayer::ModPackage, "Archive package", "Compressed package reference only.", false, false, true, true, false, false, false},
        {".rar", SupportedFileFormatLayer::ModPackage, "Archive package", "Compressed package reference only.", false, false, true, true, false, false, false},
        {".fomod", SupportedFileFormatLayer::ModPackage, "Mod installer metadata", "Mod installer/package reference class.", false, false, true, true, false, false, false},
        {".omod", SupportedFileFormatLayer::ModPackage, "Legacy mod package", "Legacy mod package reference class.", false, false, true, true, false, false, false},
    };
    return formats;
}

const SupportedFileFormat* FindSupportedFileFormat(std::string_view extension) {
    const std::string normalized = NormalizeFileExtension(extension);
    const auto& formats = SupportedFileFormats();
    const auto it = std::find_if(formats.begin(), formats.end(), [&](const SupportedFileFormat& format) {
        return format.extension == normalized;
    });
    return it == formats.end() ? nullptr : &*it;
}

std::string BuildSupportedFileFormatRegistryReport() {
    std::ostringstream report;
    report << "Supported file format registry:\n";
    for (const auto& format : SupportedFileFormats()) {
        report << "- " << format.extension << " :: " << SupportedFileFormatLayerLabel(format.layer)
               << " :: " << format.label;
        if (format.canonicalAuthoringWorld) {
            report << " :: canonical authoring world";
        }
        if (format.bunkerNative) {
            report << " :: bunker-native";
        }
        if (format.packageFormat) {
            report << " :: package format";
        }
        if (format.referenceOnly) {
            report << " :: reference-only";
        }
        if (format.futureExtractorCandidate) {
            report << " :: future extractor candidate";
        }
        if (format.executableDanger) {
            report << " :: dangerous executable/native plugin";
        }
        report << '\n';
    }
    return report.str();
}

const char* ExternalDataImportModeLabel(ExternalDataImportMode mode) {
    switch (mode) {
    case ExternalDataImportMode::NativeWorldSource:
        return "Load Native World";
    case ExternalDataImportMode::BunkerPackageReference:
        return "Stage Bunker Package Reference";
    case ExternalDataImportMode::MetadataTextSource:
        return "Metadata / text source";
    case ExternalDataImportMode::TextScriptSource:
        return "Script source reference";
    case ExternalDataImportMode::ReferenceOnly:
        return "Attach As External Reference";
    case ExternalDataImportMode::UnknownReference:
    default:
        return "Unknown external reference";
    }
}

namespace {

bool IsTextReadableExtension(std::string_view normalizedExtension) {
    return normalizedExtension == ".bwld" ||
        normalizedExtension == ".bmanifest" ||
        normalizedExtension == ".bcluster" ||
        normalizedExtension == ".json" ||
        normalizedExtension == ".xml" ||
        normalizedExtension == ".ini" ||
        normalizedExtension == ".toml" ||
        normalizedExtension == ".yaml" ||
        normalizedExtension == ".yml" ||
        normalizedExtension == ".txt" ||
        normalizedExtension == ".log" ||
        normalizedExtension == ".csv" ||
        normalizedExtension == ".tsv" ||
        normalizedExtension == ".psc";
}

ExternalDataImportMode ClassifyExternalDataImportMode(std::string_view normalizedExtension) {
    if (normalizedExtension == ".bwld") {
        return ExternalDataImportMode::NativeWorldSource;
    }
    if (normalizedExtension == ".dba" ||
        normalizedExtension == ".bmanifest" ||
        normalizedExtension == ".bcluster") {
        return ExternalDataImportMode::BunkerPackageReference;
    }
    if (normalizedExtension == ".psc") {
        return ExternalDataImportMode::TextScriptSource;
    }
    if (IsTextReadableExtension(normalizedExtension)) {
        return ExternalDataImportMode::MetadataTextSource;
    }
    if (FindSupportedFileFormat(normalizedExtension) != nullptr) {
        return ExternalDataImportMode::ReferenceOnly;
    }
    return ExternalDataImportMode::UnknownReference;
}

std::vector<std::filesystem::path> ExportDataCandidatesFor(const std::filesystem::path& startPath) {
    std::vector<std::filesystem::path> candidates;
    auto pushCandidate = [&](std::filesystem::path candidate) {
        if (candidate.empty()) {
            return;
        }
        candidate = candidate.lexically_normal();
        if (std::find(candidates.begin(), candidates.end(), candidate) == candidates.end()) {
            candidates.push_back(std::move(candidate));
        }
    };

    std::filesystem::path anchor = startPath.empty() ? std::filesystem::current_path() : startPath;
    anchor = anchor.lexically_normal();
    if (!anchor.empty() && anchor.has_filename() && anchor.filename() == "Export_data") {
        pushCandidate(anchor);
    } else {
        pushCandidate(anchor / "Export_data");
    }

    std::filesystem::path cursor = anchor;
    for (int depth = 0; depth < 4 && !cursor.empty(); ++depth) {
        pushCandidate(cursor / "Export_data");
        if (cursor.has_parent_path()) {
            cursor = cursor.parent_path();
        } else {
            break;
        }
    }

    return candidates;
}

}  // namespace

std::filesystem::path ResolveExportDataDirectory(const std::filesystem::path& startPath) {
    const auto candidates = ExportDataCandidatesFor(startPath);
    for (const auto& candidate : candidates) {
        std::error_code ec;
        if (std::filesystem::exists(candidate, ec) && std::filesystem::is_directory(candidate, ec)) {
            return candidate;
        }
    }
    return candidates.empty() ? std::filesystem::path("Export_data") : candidates.front();
}

bool CreateExportDataDirectory(const std::filesystem::path& startPath, std::filesystem::path& createdPath) {
    createdPath = ResolveExportDataDirectory(startPath);
    std::error_code ec;
    if (std::filesystem::exists(createdPath, ec)) {
        return std::filesystem::is_directory(createdPath, ec);
    }
    return std::filesystem::create_directories(createdPath, ec);
}

ExternalDataScanSummary ScanExportDataDirectory(const std::filesystem::path& startPath) {
    ExternalDataScanSummary summary;
    summary.folderPath = ResolveExportDataDirectory(startPath);

    std::error_code ec;
    summary.exists = std::filesystem::exists(summary.folderPath, ec) && std::filesystem::is_directory(summary.folderPath, ec);
    if (!summary.exists) {
        return summary;
    }

    for (const auto& entry : std::filesystem::directory_iterator(summary.folderPath, ec)) {
        if (ec || !entry.is_regular_file()) {
            continue;
        }

        ExternalDataFileRecord record;
        record.path = entry.path();
        record.fileName = entry.path().filename().string();
        record.extension = NormalizeFileExtension(entry.path().extension().string());
        record.importMode = ClassifyExternalDataImportMode(record.extension);
        record.textReadable = IsTextReadableExtension(record.extension);
        record.referenceOnly = record.importMode == ExternalDataImportMode::ReferenceOnly;

        if (const auto* format = FindSupportedFileFormat(record.extension)) {
            record.recognized = true;
            record.layerLabel = SupportedFileFormatLayerLabel(format->layer);
            record.formatLabel = format->label;
            record.bunkerNative = format->bunkerNative;
            record.canonicalAuthoringWorld = format->canonicalAuthoringWorld;
            record.packageFormat = format->packageFormat;
            record.referenceOnly = format->referenceOnly || record.importMode == ExternalDataImportMode::ReferenceOnly ||
                record.importMode == ExternalDataImportMode::BunkerPackageReference;
            record.executableDanger = format->executableDanger;
            record.textReadable = format->canBeTextPreviewed || record.textReadable;
            ++summary.recognizedFileCount;
            if (record.extension == ".bwld") {
                summary.bwldPresent = true;
            }
        } else {
            record.layerLabel = "Unknown";
            record.formatLabel = "Unknown external reference";
            record.referenceOnly = false;
            ++summary.unknownFileCount;
        }

        summary.files.push_back(std::move(record));
    }

    std::sort(summary.files.begin(), summary.files.end(), [](const ExternalDataFileRecord& lhs, const ExternalDataFileRecord& rhs) {
        return lhs.fileName < rhs.fileName;
    });
    summary.foundFileCount = summary.files.size();
    return summary;
}

std::string BuildExternalDataScanReport(const ExternalDataScanSummary& summary) {
    std::ostringstream report;
    report << "Export_data folder: " << summary.folderPath.string() << '\n';
    report << "Export_data exists: " << (summary.exists ? "yes" : "no") << '\n';
    report << "Export_data files: " << summary.foundFileCount << '\n';
    report << "Export_data recognized: " << summary.recognizedFileCount << '\n';
    report << "Export_data unknown: " << summary.unknownFileCount << '\n';
    report << "Export_data .bwld present: " << (summary.bwldPresent ? "yes" : "no") << '\n';
    if (!summary.files.empty()) {
        report << "Export_data entries:\n";
        for (const auto& file : summary.files) {
            report << "- " << file.fileName
                   << " :: " << file.extension
                   << " :: " << file.layerLabel
                   << " :: " << file.formatLabel
                   << " :: " << ExternalDataImportModeLabel(file.importMode);
            if (file.bunkerNative) {
                report << " :: bunker-native";
            }
            if (file.canonicalAuthoringWorld) {
                report << " :: canonical authoring world";
            }
            if (file.packageFormat) {
                report << " :: package format";
            }
            if (file.referenceOnly) {
                report << " :: reference-only";
            }
            if (file.executableDanger) {
                report << " :: dangerous executable";
            }
            report << '\n';
        }
    }
    return report.str();
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

std::string BuildWorldValidationReport(
    const World& world,
    const std::vector<ValidationIssue>& issues,
    const WorldExportResult& result) {
    std::ostringstream report;
    std::vector<PrefabRecord> prefabs;
    const bool prefabLibraryLoaded = LoadPrefabLibrary(prefabs);
    const auto brokenPrefabReferenceIndices = CollectBrokenPrefabReferenceObjectIndices(world, prefabs);
    const auto layerNames = world.CollectEditorLayerNames();
    int semanticObjectCount = 0;
    int scalableLootObjectCount = 0;
    std::size_t scalableLootEntryCount = 0;
    std::size_t nonEmptyScalableLootEntryCount = 0;
    int manualLootObjectCount = 0;
    int randomLootObjectCount = 0;
    for (const auto& object : world.objects) {
        if (!object.scriptTag.empty()) {
            ++semanticObjectCount;
        }
        if (!object.lootEntries.empty()) {
            ++scalableLootObjectCount;
            if (object.lootMode == LootMode::RandomTable) {
                ++randomLootObjectCount;
            } else {
                ++manualLootObjectCount;
            }
            scalableLootEntryCount += object.lootEntries.size();
            for (const auto& entry : object.lootEntries) {
                if (!entry.itemId.empty()) {
                    ++nonEmptyScalableLootEntryCount;
                }
            }
        }
    }

    report << "World Validation Report\n";
    report << "Generated: " << result.generatedAt << '\n';
    report << "Target: " << result.worldPath.string() << '\n';
    report << "Format: " << CurrentWorldBinaryFormatLabel() << '\n';
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
    report << "Objects: " << world.objects.size() << '\n';
    report << "Semantic objects: " << semanticObjectCount << '\n';
    report << "Scalable loot objects: " << scalableLootObjectCount << '\n';
    report << "Scalable loot entries: " << scalableLootEntryCount << '\n';
    report << "Non-empty scalable loot entries: " << nonEmptyScalableLootEntryCount << '\n';
    report << "Loot mode manual objects: " << manualLootObjectCount << '\n';
    report << "Loot mode random objects: " << randomLootObjectCount << '\n';
    report << "Layers: " << layerNames.size() << '\n';
    report << "Prefab-derived objects: " << CountPrefabDerivedObjects(world) << '\n';
    report << "Broken prefab references: " << brokenPrefabReferenceIndices.size() << '\n';
    report << "Prefab library entries: " << (prefabLibraryLoaded ? prefabs.size() : 0) << '\n';
    report << "Prefab library loaded: " << (prefabLibraryLoaded ? "yes" : "no") << '\n';
    if (const auto* targetFormat = FindSupportedFileFormat(result.worldPath.extension().string())) {
        report << "Target extension class: " << SupportedFileFormatLayerLabel(targetFormat->layer) << '\n';
        report << "Target extension label: " << targetFormat->label << '\n';
        report << "Target extension canonical authoring world: " << (targetFormat->canonicalAuthoringWorld ? "yes" : "no") << '\n';
    } else {
        report << "Target extension class: Unknown\n";
        report << "Target extension label: Unknown extension, report-only artifact handling\n";
        report << "Target extension canonical authoring world: no\n";
    }
    report << "Message: " << result.message << '\n';
    report << BuildSupportedFileFormatRegistryReport();
    report << BuildExternalDataScanReport(ScanExportDataDirectory());

    if (!layerNames.empty()) {
        report << "Layer names:\n";
        for (const auto& layerName : layerNames) {
            report << "- " << layerName << '\n';
        }
    }

    if (!brokenPrefabReferenceIndices.empty()) {
        report << "Broken prefab reference objects:\n";
        for (const int objectIndex : brokenPrefabReferenceIndices) {
            if (objectIndex < 0 || objectIndex >= static_cast<int>(world.objects.size())) {
                continue;
            }
            const auto& object = world.objects[static_cast<std::size_t>(objectIndex)];
            report << "- " << object.registryId << " :: prefabSourceId=" << object.prefabSourceId << '\n';
        }
    }

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
            reportFile << BuildWorldValidationReport(world, issues, result);
        }
    }
    AppendExportAuditEntry(result);
    return result;
}

}  // namespace bunker
