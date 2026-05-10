# PROJECT_FINISH_DIRECTIVE_FOR_CLI_AGENT.md

**Project:** `Bunker Protocol` / `RobertoGol/GMyGameDoNotTouch`  
**Target:** finish the base playable game / `v0.1-showable`  
**Audience:** coding CLI agent working directly inside the repository  
**Date:** 2026-04-25  
**Primary rule:** finish the project without touching the map.

---

# 0. READ THIS FIRST — ABSOLUTE EXECUTION CONTRACT

You are a coding agent operating inside the repository.

You must treat this file as the main execution directive for finishing the project.

You are not allowed to skip this file, summarize it away, or replace it with your own roadmap.

Your job is not to brainstorm. Your job is to finish the existing project to a clean playable `v0.1-showable` state.

The human developer will make the map manually. You must not make, reshape, author, decorate, populate, or redesign the map.

The project already has many systems. Do not restart it. Do not create a second project. Do not replace the architecture. Do not create a new engine layer unless a tiny glue layer is required and proven by code.

You must work in small, compile-conscious patches.

IMPORTANT BUILD AUTHORITY RULE:

The human developer does the real build and test run manually. Do **not** trust your own build result as final truth. Do **not** claim the project builds unless the human supplies build output proving it.

After each meaningful patch:

1. do **not** run the final build as proof of success;
2. perform static/self verification only: inspect changed code, headers, includes, CMake source lists, obvious syntax/API mismatches, and `git diff`;
3. ensure no forbidden map/content files were modified;
4. write exact manual build/test commands for the human developer;
5. update only the relevant docs/checklists.

You may run read-only inspection commands. You may run lightweight formatting/diff commands. You must not treat `cmake --build`, `ctest`, or IDE build output from your environment as authoritative. If the human explicitly asks you to run a build command anyway, report it as "local agent attempt only, human must verify".

If a task appears already implemented, verify it in code and tests, then mark it as done or leave no-op notes. Do not duplicate systems.

If a task conflicts with the current code, current code wins. Adapt this directive to the code, not the code to imagined APIs.

---

# 0.1 HUMAN BUILD AUTHORITY — DO NOT TRUST THE AGENT BUILD

The human developer said: **"do not trust the CLI agent with the build; I will build it myself."**

Therefore:

- The CLI agent must write and modify code, tests, docs, and validation logic.
- The CLI agent must not use its own build result as final proof.
- The CLI agent must not say "build passed", "tests passed", "release is verified", or "done" based only on its own environment.
- The CLI agent must provide clear commands for the human to run locally.
- The CLI agent must stop after patches with a **Human Build Handoff** block.
- The human build result is the only authoritative build result.

Allowed agent-side checks:

```bash
git status --short
git diff --check
git diff --name-only
rg -n "TODO|FIXME|throw std::runtime_error|assert\(" include src tests Launcher Editor 2>/dev/null || true
```

Forbidden as proof of completion:

```bash
cmake --build ...
ctest ...
msbuild ...
ninja ...
make ...
```

Those commands may appear in documentation or in a handoff block for the human, but the CLI agent must not use them as authoritative completion evidence.
---

# 1. HARD SAFETY / LEGAL / ASSET BOUNDARY

## 1.1 Do not use copyrighted game assets without permission

Do **not** implement instructions for extracting, copying, converting, or using copyrighted assets from Fallout 4, `.ba2` archives, Bethesda games, or any other third-party commercial game.

Do **not** add:

- `.ba2` extractors;
- asset ripping scripts;
- hardcoded paths to Fallout 4 installations;
- references to Bethesda/Fallout asset names as required runtime assets;
- copied textures, meshes, sounds, materials, or archives;
- instructions that tell the user how to obtain such assets.

This project must be finishable with:

- original assets;
- legally licensed assets;
- public-domain / CC0 assets;
- developer-created assets;
- local placeholder assets supplied by the developer;
- procedural placeholders;
- simple debug geometry and neutral materials.

The game may support a generic local asset override folder, but it must not assume or require infringing assets.

Correct implementation pattern:

```text
assets/
  placeholder/
  original/
  licensed/
  local_override/   # ignored by git, user-owned, not required by build/tests
```

Incorrect implementation pattern:

```text
Fallout4/Data/Fallout4 - Textures.ba2
Bethesda asset extractor
rip_fo4_textures.py
hardcoded FO4 material database
```

If existing code already contains questionable asset references, do not expand them. Replace with neutral generic naming where possible.

## 1.2 Map ownership boundary

The human developer owns the map.

You must not edit authored map data, map layouts, room placement, route geometry, decorative placement, terrain, city layout, bunker layout, authored encounters, or any other hand-made level-design content.

You may edit systems that validate, load, save, reference, or report requirements for maps.

You may add semantic anchor requirements and validation reports.

You may add placeholder fallback behavior that allows tests to run without a final authored map.

You must not change actual map content.

Forbidden examples:

```text
maps/**
worlds/**
content/maps/**
content/worlds/**
assets/maps/**
assets/worlds/**
*.world
*.map
*.level
*.scene
*.prefab       # only if it is authored map content
```

Allowed examples:

```text
include/WorldValidation.hpp
src/WorldValidation.cpp
include/WorldSemanticAuthoring.hpp
src/WorldSemanticAuthoring.cpp
tests/*Smoke*.cpp
Launcher/src/*
src/GameRuntime*.cpp
include/StoryRoute.hpp
src/StoryRoute.cpp
```

If this repository uses different names, detect the actual map/content paths with `find`, `git ls-files`, and existing code. Then preserve them.

Before every commit, run:

```bash
git diff --name-only
```

If any map/content file changed, stop and revert that file unless the human explicitly asked for that exact file.

---

# 2. CURRENT PROJECT TRUTH

The project is named `Bunker Protocol`.

The expected application split is:

- `BunkerLauncher`
- `BunkerGame`
- `BunkerEditor`
- `BunkerSmokeChecks`

The baseline form is:

- solo + LAN first;
- launcher as user entry point;
- editor as separate production tool;
- runtime as separate gameplay application.

Core pillars:

- `BT-72`
- authored world
- bunker-to-surface progression
- recovery
- industry
- logistics
- persistence
- service / support loops
- `Pip-Pad` as the primary system interface

Base game goal:

- cryo wake / bunker start;
- early access card / clearance logic;
- Pip-Pad acquisition;
- archive/data trail;
- BT-72 discovery and restoration;
- sync/link;
- hangar tutorial;
- bunker exit / surface arrival;
- first heavy clearance;
- first combat;
- first service/rest;
- first recovery payoff;
- debrief;
- handoff into recovery / industry / logistics backbone.

The current canon explicitly says not to open large new parallel branches. Finish the base game.

---

# 3. PRIME DIRECTIVE

Finish `v0.1-showable`.

This means:

1. the repository is prepared so the human can build it cleanly;
2. smoke checks are prepared so the human can run and verify them;
3. the launcher can present a selected world/profile state;
4. the runtime can execute the first playable route using semantic anchors and route states;
5. BT-72 has readable early gameplay impact;
6. first combat has understandable feedback;
7. recovery/industry/logistics has a clear first payoff and next hook;
8. save/load keeps route/profile/world/service state synchronized;
9. Pip-Pad and launcher describe the same truth;
10. editor validation helps the human map author connect the map without the agent touching the map;
11. docs explain how the human builds, runs, tests, and packages the project.

The release target is not a commercial full game. The release target is a coherent, local, showable vertical slice that proves the base game loop.

---

# 4. ABSOLUTE NON-GOALS

Do not do these:

- do not make the map;
- do not edit authored layout;
- do not place rooms;
- do not decorate spaces;
- do not create a new surface city layout;
- do not copy recognizable vault/bunker designs from other games;
- do not create DLC branches;
- do not create MMO/backend architecture;
- do not build player-side world editor mode;
- do not turn Camp/AIMP into the map editor;
- do not turn BT-72 into a finished free tank from the start;
- do not turn Lanline into a separate internet product;
- do not add a pay-to-win economy;
- do not rewrite the whole project;
- do not invent new architecture when existing code has the system;
- do not remove existing smoke coverage;
- do not replace CMake with another build system;
- do not add heavyweight dependencies unless unavoidable;
- do not add copyrighted assets or asset extraction tools.

---

# 5. DEVELOPMENT METHOD FOR CLI AGENT

## 5.1 First command sequence

Run this from repository root:

```bash
pwd
git status --short
git branch --show-current
find . -maxdepth 3 -type f | sort | sed 's#^./##' | head -300
```

Then inspect the main docs and build files:

```bash
sed -n '1,220p' PROJECT_CANON_AND_STATUS.md 2>/dev/null || true
sed -n '1,220p' Next.md 2>/dev/null || true
sed -n '1,220p' Next_compact.md 2>/dev/null || true
sed -n '1,220p' ROADMAP.md 2>/dev/null || true
sed -n '1,260p' CMakeLists.txt 2>/dev/null || true
```

Inspect system files by name, if present:

```bash
find include src tests Launcher Editor -type f 2>/dev/null | sort | grep -E 'StoryRoute|GameRuntime|World|Validation|Semantic|Progression|SessionProfiles|Lanline|Hangar|Pip|Tank|BT|Smoke|Combat|Recovery|Industry|Logistics|Service|Launcher|Editor' || true
```

Search for current route terms:

```bash
rg -n "cryo|Pip|BT-72|BT72|hangar|surface|combat|recovery|industry|logistics|debrief|route|semantic|anchor|validation|service|Lanline|profile" .
```

Do not edit anything until you have inspected the current code.

## 5.2 Branch

Create or use a finishing branch:

```bash
git checkout -b finish/v0.1-showable 2>/dev/null || git checkout finish/v0.1-showable
```

## 5.3 Build discovery and human build handoff

Do **not** use your own build as authoritative proof. The human developer builds manually.

Your job is to keep the repository buildable by inspection and to provide exact commands for the human.

Human manual CMake debug build:

```bash
cmake -S . -B build_finish_debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build_finish_debug --parallel
ctest --test-dir build_finish_debug --output-on-failure
```

Human manual Visual Studio/MSVC build:

```bash
cmake -S . -B build_finish_msvc -G "Visual Studio 17 2022" -A x64
cmake --build build_finish_msvc --config Debug --parallel
ctest --test-dir build_finish_msvc -C Debug --output-on-failure
```

If the repo already has `build_verify_ninja` or similar directories, do not rely on them blindly. Tell the human to prefer a fresh build directory.

Agent-side static checks after edits:

```bash
git status --short
git diff --check
git diff --name-only
```

If CMake source lists changed, inspect `CMakeLists.txt` manually and make sure every new `.cpp` file is listed in the correct target.

## 5.4 Patch size

Work in small patches.

Good patch:

```text
feat(route): add v0.1 semantic anchor contract
```

Bad patch:

```text
rewrite entire game
```

## 5.5 Verification after each patch

Run only static/self checks unless the human explicitly instructed otherwise:

```bash
git diff --check
git diff --name-only
git status --short
```

Then produce this handoff block:

```text
HUMAN BUILD HANDOFF
Changed files:
- ...

Agent static checks:
- git diff --check: clean / issue listed
- forbidden map/content files changed: no / list
- obvious CMake source list updates: done / not needed / needs human check

Please run manually:
cmake -S . -B build_finish_debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build_finish_debug --parallel
ctest --test-dir build_finish_debug --output-on-failure

If using MSVC:
cmake -S . -B build_finish_msvc -G "Visual Studio 17 2022" -A x64
cmake --build build_finish_msvc --config Debug --parallel
ctest --test-dir build_finish_msvc -C Debug --output-on-failure
```

If build/test commands differ, detect the correct ones from `CMakeLists.txt` and print them for the human. Do not claim they passed unless the human reports the result.

## 5.6 No silent failures

If the human reports a build/test failure:

1. ask for or use the exact failing command and first meaningful compiler/test error;
2. fix the smallest cause;
3. provide a new patch and a new human build handoff.

Do not hide failures in docs. Do not mark a task complete until the human confirms build/tests passed.

---

# 6. MAP LOCKDOWN PROTOCOL

Before editing, create a baseline list of files that look like authored content:

```bash
mkdir -p .finish_audit
git ls-files | grep -Ei '(^|/)(map|maps|world|worlds|level|levels|scene|scenes|terrain|tiles|prefabs|layout|layouts|content|assets)(/|$)|\.(map|world|level|scene|terrain|prefab|tmx|tsx|json)$' > .finish_audit/potential_authored_content_files.txt || true
```

This list may include system files. Inspect it. Do not blindly modify listed files.

After each patch:

```bash
git diff --name-only > .finish_audit/changed_files.txt
comm -12 <(sort .finish_audit/potential_authored_content_files.txt) <(sort .finish_audit/changed_files.txt) || true
```

If any authored map/content file appears, revert it unless it is a code/system file and clearly safe.

Allowed editor work:

- validation logic;
- semantic anchor diagnostics;
- export error messages;
- missing-anchor reports;
- UI text for validation results.

Forbidden editor work:

- changing the actual map;
- adding rooms;
- placing objects;
- changing authored layout;
- moving anchors in existing authored content.

---

# 7. RELEASE DEFINITION OF DONE

`v0.1-showable` is done only when all of the following are true.

## 7.1 Build — human verified only

The following are required for `v0.1-showable`, but only the human developer can verify them:

- `BunkerGame` builds.
- `BunkerLauncher` builds.
- `BunkerEditor` builds.
- `BunkerSmokeChecks` builds.
- `ctest` or equivalent smoke command passes.

The CLI agent may prepare code for these outcomes but must not claim them as passed without human-provided output.

## 7.2 First route

The game can represent this route, even if the final authored map is not finished:

```text
cryo_wake
emergency_melee_pickup
bunker_access_card
pip_pad_acquisition
archive_trail
bt72_hull_discovery
bt72_core_discovery
bt72_service_notes
bt72_restoration_started
bt72_core_installed
bt72_sync_linked
hangar_tutorial
bunker_exit_unlocked
surface_arrival
heavy_clearance
first_combat
first_service_rest
first_recovery_node
debrief
industrial_handoff
```

The exact enum names may differ. Use existing route/state naming where possible.

## 7.3 Map integration

- The route uses semantic anchors / world nodes / descriptors, not hardcoded map geometry.
- Validation can report missing required v0.1 anchors.
- Missing anchors produce a readable error/warning.
- Placeholder/test worlds can satisfy route smoke checks without editing the real map.
- The human map author can add anchors later.

## 7.4 BT-72

- BT-72 is not available as a completed tank at start.
- BT-72 is discovered, restored, activated, and linked.
- BT-72 has readable first combat impact.
- Heavy shot feedback is visible in runtime/HUD/Pip-Pad or equivalent text UI.
- Second-seat/gunner policy does not conflict with profile/runtime state.

## 7.5 Combat / RPG

- First combat is understandable.
- Enemy awareness does not behave like wallhack.
- Heavy BT-72 hit has clear effect.
- SPECIAL/skills/service doctrine or equivalent early RPG weighting affects first combat/service at least minimally.
- Mechanical hostile modular damage, if already present, remains covered by smoke checks.

## 7.6 Recovery / industry / logistics

- First recovery node is not a dead end.
- Debrief hands off into recovery/industry/logistics.
- Launcher/Pip-Pad/runtime use the same backbone summary.
- Save/load preserves the handoff state.

## 7.7 Launcher / services / profile

- Launcher displays selected world/profile route status.
- Launcher does not show a stale different world summary.
- Services/Lanline preview, if present, mirrors the same route/recovery truth.
- Profile/save-load is deterministic enough for smoke tests.

## 7.8 Docs

- `README` or release notes explain build/run/test.
- `PROJECT_CANON_AND_STATUS.md` remains canon, not a noisy task dump.
- `Next.md` contains only active unfinished work.
- `ROADMAP.md` records final checkpoint status.
- No docs say copyrighted assets are required.

---

# 8. RECOMMENDED COMMIT PLAN

Use these commits if they map cleanly to the current code.

Do not force commit names if the repository style differs.

```text
chore: audit v0.1 finish surface
chore(build): prepare clean debug smoke build for human verification
feat(route): add v0.1 semantic anchor contract
feat(validation): report missing first-route anchors without touching map
feat(runtime): harden first-route state transitions
feat(pippad): mirror route and recovery objectives
feat(bt72): polish first combat readability hooks
feat(combat): lock first encounter smoke expectations
feat(recovery): finish industrial handoff summary
feat(launcher): sync selected-world v0.1 preview
feat(assets): add legal placeholder asset provider
test: expand v0.1 showable smoke checks
docs: add v0.1 build run release notes
chore: cut v0.1-showable checklist
```

If a commit is already done, do not duplicate it. Verify and skip.

---

# 9. TASK A — COMPILE-READINESS AND HUMAN BUILD HANDOFF

## Objective

Prepare the repository so the human can establish a clean build/test baseline. Do not use the CLI agent build as final truth.

## Agent commands

```bash
git status --short
git diff --check
git diff --name-only
sed -n '1,260p' CMakeLists.txt 2>/dev/null || true
```

## Human commands to print after the patch

```bash
cmake -S . -B build_finish_debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build_finish_debug --parallel
ctest --test-dir build_finish_debug --output-on-failure
```

If the human reports failure, fix build/test failures before feature work continues.

## Rules

- Do not suppress tests.
- Do not delete failing tests unless proven obsolete and replaced.
- Do not mark tests as ignored to make the build green.
- Do not patch generated build folders.
- Do not edit map files to make tests pass.

## Likely fixes

- missing includes;
- stale function signatures;
- CMake target source list mismatch;
- tests expecting old route step names;
- inconsistent enum/string conversion;
- serialization default mismatch;
- path handling inconsistency;
- Windows/MSVC warnings-as-errors issues;
- missing `std::` includes.

## Acceptance

- all main targets compile;
- smoke test executable is configured for the human to run;
- any human-reported failures are real game issues, not build-system rot;
- no map/content files modified.

---

# 10. TASK B — V0.1 SEMANTIC ANCHOR CONTRACT

## Objective

Make the first playable route independent from the final map layout.

The map author will place anchors later. The code must know which anchors are required and report what is missing.

## Required semantic anchors

Use existing naming if the code already has anchors. Otherwise introduce names similar to these:

```text
cryo_start
emergency_melee_pickup
first_vermin_gate
bunker_access_card
pip_pad_pickup
archive_trail_start
archive_trail_end
engineering_entry
bt72_hull
bt72_core
bt72_service_notes
bt72_material_cache
bt72_restoration_bay
bt72_sync_station
hangar_tutorial
bunker_exit_gate
surface_arrival
heavy_clearance_blocker
first_combat_zone
first_service_bay
first_recovery_node
debrief_terminal
industrial_handoff_node
```

Do not put coordinates here unless existing code requires it. Prefer symbolic IDs.

## Implementation guidance

Find existing files:

```bash
rg -n "Semantic|Anchor|WorldValidation|Descriptor|WorldNode|StoryRoute|Route" include src tests Editor Launcher
```

If the project already has `WorldSemanticAuthoring` or `WorldValidation`, extend them.

If not, create minimal code in the existing style:

- a list of required v0.1 route anchors;
- a validator that receives a world descriptor and returns missing anchors;
- readable diagnostic text;
- smoke tests for complete and incomplete anchor sets.

Do not create a huge framework.

## Example data shape

Adapt to current C++ style.

```cpp
struct RouteAnchorRequirement {
    std::string id;
    std::string label;
    std::string reason;
    bool requiredForV01 = true;
};
```

```cpp
struct RouteAnchorValidationResult {
    bool ok = false;
    std::vector<std::string> missingRequiredAnchors;
    std::vector<std::string> warnings;
};
```

Do not use this exact code if existing types already solve it.

## Acceptance

- a complete test descriptor passes validation;
- a descriptor missing `bt72_core` fails with a clear missing-anchor message;
- a descriptor missing `first_recovery_node` fails with a clear missing-anchor message;
- runtime/launcher/editor can display or log missing anchor diagnostics;
- no authored map data changed.

---

# 11. TASK C — STORY ROUTE HARDENING

## Objective

The first route must advance consistently without depending on final map geometry.

## Route beat order

The canonical route order is:

```text
cryo wake
first melee / vermin survival
access card / clearance
early Pip-Pad anticipation
Pip-Pad acquisition
archive/data trail
engineering layer
BT-72 hull discovery
BT-72 core discovery
service notes / materials
staged restoration
core install
sync/link
hangar tutorial
bunker exit unlock
surface arrival
heavy clearance
first combat
first service/rest
first recovery node
debrief
industrial handoff
```

## What to inspect

```bash
rg -n "StoryRoute|RouteStep|Objective|Checkpoint|Progression|Debrief|Industrial|Recovery" include src tests
```

## Required behavior

- Late route state must not regress because an earlier optional flag is missing.
- Save/load must preserve current route state.
- Launcher summary and runtime objective must describe the same current route truth.
- Pip-Pad objective must not disagree with runtime route state.
- Route events must remain locked before onboarding/debrief where appropriate.

## Anti-regression rule

If player has reached `industrial_handoff`, do not show `pick up Pip-Pad` as active objective.

If player has reached `surface_arrival`, do not regress to `bunker_exit_unlocked` only because one presentation flag did not save.

If player has completed `first_combat`, do not show first combat as pending after save/load.

## Acceptance

Add or verify smoke checks:

```text
route_does_not_regress_from_surface_arrival
route_does_not_regress_from_debrief
route_summary_matches_runtime_after_save_load
pippad_objective_matches_story_route
launcher_preview_matches_selected_world
```

Use actual test naming conventions from the repository.

---

# 12. TASK D — PIP-PAD OBJECTIVE AND UI TRUTH

## Objective

Pip-Pad should be the player-facing system interface for objective, route, service, recovery, and incident summaries.

## Required player-readable objectives

Use short readable strings. Avoid coordinates.

Examples:

```text
Wake and stabilize the cryo bay.
Find an emergency weapon.
Recover a valid access card.
Locate the Pip-Pad interface.
Follow the archive trail into engineering.
Find the BT-72 hull.
Recover the BT-72 core.
Restore the BT-72 in stages.
Link with the BT-72.
Use the hangar systems to open the exit route.
Reach the surface.
Clear the heavy obstruction.
Survive the first combat contact.
Service the BT-72 and recover.
Secure the first recovery node.
Debrief and unlock the industrial backbone.
```

Use existing localization/string style if present.

## Required consistency

Pip-Pad must read route/recovery truth from the same source as runtime and launcher.

Do not hardcode a separate objective ladder inside Pip-Pad if `StoryRoute` already owns the truth.

## Acceptance

- every route stage has a non-empty Pip-Pad objective;
- Pip-Pad summary after debrief mentions recovery/industry/logistics;
- Pip-Pad does not show unavailable merchant/service UI before unlock;
- tests cover at least pre-Pip-Pad, BT-72 restoration, surface arrival, post-debrief states.

---

# 13. TASK E — BT-72 EARLY GAMEPLAY POLISH

## Objective

Make BT-72 feel like the central early-game companion platform without expanding scope.

## Required truth

BT-72:

- is discovered as hull + core, not given complete;
- requires staged restoration;
- requires sync/link;
- supports pilot role;
- supports optional second seat / gunner policy;
- creates first combat contrast between пешком and platform gameplay;
- needs service/rest after first combat;
- connects to recovery/industry/logistics backbone.

## Do not add

- giant tank upgrade tree;
- dozens of weapons;
- full vehicle physics rewrite;
- open-world driving system if not already present;
- complex crew AI beyond existing scope;
- new map routes.

## First combat readability

Ensure the player can understand:

- when the BT-72 fires a heavy shot;
- what the shot hit;
- whether enemy sensors/weapon/mobility were affected if modular damage exists;
- whether the player is pilot/gunner/solo;
- when service is needed after combat.

## Possible implementation surfaces

Inspect:

```bash
rg -n "BT-72|BT72|Tank|Hangar|Service|Gunner|Seat|Heavy|Muzzle|Shock|Damage|Mechanical|Robot" include src tests Launcher Editor
```

Likely places:

```text
include/HangarSystem.hpp
src/HangarSystem.cpp
include/GameRuntime.hpp
src/GameRuntime.cpp
include/GameRuntimePipPad.hpp
src/GameRuntimePipPad.cpp
include/Progression.hpp
src/Progression.cpp
include/StoryRoute.hpp
src/StoryRoute.cpp
tests/*Smoke*.cpp
```

Use actual repository files.

## Acceptance

- BT-72 restoration state is visible in route/Pip-Pad/runtime;
- BT-72 sync/link state persists;
- first combat summary includes BT-72 effect;
- service/rest handoff triggers after first combat;
- second-seat policy smoke tests pass or are added if missing;
- no map edited.

---

# 14. TASK F — FIRST COMBAT / RPG DEPTH WITHOUT SCOPE EXPLOSION

## Objective

The first combat must have enough depth to show the RPG/combat direction, but not become a full combat rewrite.

## Required minimum

- enemy perception is not wallhack;
- blocked line-of-sight matters if the system exists;
- mechanical enemies have readable damage state if already implemented;
- SPECIAL/skills/doctrine/service choices influence outcome or feedback;
- BT-72 heavy shot has readable tactical effect;
- combat result feeds first service/rest and route state.

## Human/ghoul/robot behavior boundary

Use this as design truth if enemies exist:

```text
humans = tactical + self-preservation
ghouls = rush melee
robots = patterns + zone control + modular mechanical damage
```

For `v0.1`, only implement what is necessary for the first encounter and smoke tests.

## Do not add

- full faction AI;
- full stealth sim;
- giant weapon catalogue;
- huge ballistics rewrite;
- always-on friendly fire;
- fire propagation;
- total destruction sandbox;
- full crowd simulation.

## Acceptance

Smoke checks should prove:

```text
first combat can start
first combat can resolve
BT-72 heavy shot changes encounter feedback
service/rest unlocks after first combat
route advances to recovery node after service/rest
save/load preserves combat result
```

---

# 15. TASK G — RECOVERY / INDUSTRY / LOGISTICS BACKBONE

## Objective

After first recovery node and debrief, the game must clearly point into mid-game systems.

This is not a complete mid-game. It is a believable backbone handoff.

## Required first payoff

After first recovery node, player should understand:

- what was recovered;
- why it matters;
- what system changed;
- what the next operational goal is;
- how BT-72/service/industry/logistics are connected.

## Backbone stage examples

Use existing enum/state names if present. If not, possible stages:

```text
locked
seeded
first_node_secured
debrief_available
industrial_backbone_unlocked
logistics_preview_available
service_network_seeded
```

## Required consistency

Runtime, Pip-Pad, Launcher, and Lanline/Services must read the same recovery backbone summary.

Do not create four separate summaries.

## Suggested single truth type

Only if no existing equivalent exists:

```cpp
struct RecoveryBackboneStatus {
    std::string stage;
    std::string statusLine;
    std::string payoffLine;
    std::string nextActionLine;
    bool industryUnlocked = false;
    bool logisticsPreviewAvailable = false;
    bool serviceNetworkSeeded = false;
};
```

Adapt to existing code style.

## Acceptance

- post-debrief summary mentions industry/logistics/recovery;
- launcher selected-world preview shows the same stage;
- Pip-Pad shows same next action;
- services shell mirrors same unlock state if present;
- save/load preserves stage;
- smoke check covers handoff.

---

# 16. TASK H — LAUNCHER / LANLINE / PROFILE / SAVE-LOAD GLUE

## Objective

The launcher must not lie.

The launcher must show the selected world/profile route and recovery state, not stale global state.

## Inspect

```bash
rg -n "Launcher|SelectedWorld|WorldPreview|SessionProfile|Lanline|ServicesUnlock|Save|Load|Profile|RouteSummary|Recovery" Launcher include src tests
```

## Required behavior

- launcher selected-world summary reads world-scoped state;
- selected profile route state persists;
- runtime writes route/recovery events in the same format launcher reads;
- Lanline/Services unlock preview reads same snapshot;
- announcement/read-state widgets do not corrupt route truth;
- no stale preview when switching worlds.

## Smoke scenarios

```text
create profile A world A with early route
create profile B world B with post-debrief route
select world A: launcher shows early route
select world B: launcher shows post-debrief route
runtime advances world B recovery
launcher refresh shows updated world B recovery
world A remains unchanged
```

## Acceptance

- selected-world preview smoke test passes;
- route summary and recovery summary use one source of truth;
- save/load is deterministic;
- no map edited.

---

# 17. TASK I — EDITOR VALIDATION WITHOUT MAP AUTHORING

## Objective

The editor should help the human map author connect the first route, but the agent must not author the map.

## Allowed editor work

- add validation panel/report for missing v0.1 anchors;
- add export warning if required semantic anchors are missing;
- add readable label/reason for each required anchor;
- add docs explaining what the human must place;
- add tests for validation logic.

## Forbidden editor work

- placing anchors in actual map files;
- moving existing authored anchors;
- creating rooms;
- decorating;
- changing layout;
- changing terrain;
- creating the surface map.

## Validation report should say

For each missing anchor:

```text
[id] label — why required — route stage that needs it
```

Example:

```text
bt72_core — BT-72 core discovery point — required before staged restoration can complete.
```

## Acceptance

- editor/export can report missing v0.1 route anchors;
- report is readable;
- tests can validate a synthetic descriptor;
- no real authored map file changed.

---

# 18. TASK J — LEGAL PLACEHOLDER ASSET PROVIDER

## Objective

The game should be able to run/show the vertical slice without requiring copyrighted third-party assets.

## Required policy

Use original/placeholder/legal assets only.

Do not implement third-party game asset extraction.

## Acceptable implementation

If asset handling is currently fragile, add a small resolver layer:

```text
AssetResolver
AssetCatalog
MaterialFallbacks
PlaceholderTexture
MissingAssetDiagnostic
```

But only if needed by current code.

## Required behavior

- missing texture does not crash the game;
- missing material resolves to neutral placeholder;
- diagnostics are readable;
- local override folder is optional and ignored by git;
- no external commercial game path is required.

## Example `.gitignore` entries

Only add if relevant:

```gitignore
assets/local_override/
local_assets/
*.ba2
*.bsa
```

Do not add instructions for obtaining `.ba2` or `.bsa` files.

## Acceptance

- clean checkout is prepared so the human can build and run smoke checks without third-party assets;
- missing asset fallback is tested or manually verified;
- docs state that external copyrighted assets are not required;
- no copyrighted asset references added.

---

# 19. TASK K — SMOKE TEST EXPANSION

## Objective

Smoke checks should prove the first playable route and release glue.

## Add or verify smoke tests for

```text
build_target_exists_bunker_game
build_target_exists_bunker_launcher
build_target_exists_bunker_editor
route_required_anchors_complete_descriptor_passes
route_required_anchors_missing_bt72_core_fails
route_required_anchors_missing_recovery_node_fails
route_start_to_pippad_objective_chain
route_bt72_restoration_chain
route_surface_arrival_checkpoint
route_first_combat_to_service
route_recovery_node_to_debrief
route_debrief_to_industrial_handoff
pippad_objective_matches_route_state
launcher_selected_world_preview_matches_profile_state
lanline_services_unlock_matches_recovery_state
save_load_preserves_route_state
save_load_preserves_recovery_backbone_state
bt72_second_seat_policy_runtime_feedback
bt72_heavy_shot_feedback_present
first_combat_modular_damage_feedback_if_supported
missing_asset_uses_placeholder_if_asset_layer_exists
```

Do not add fake tests that assert only `true`.

Tests may use synthetic world descriptors and fake profiles. They must not need the final human-authored map.

## Acceptance

- smoke checks fail on missing required route anchor;
- smoke checks pass on complete synthetic route descriptor;
- smoke checks cover save/load route state;
- smoke checks cover launcher selected-world preview;
- no map edited.

---

# 20. TASK L — RELEASE DOCUMENTATION

## Objective

Make the project understandable to build, run, and continue.

## Required docs

Create or update docs with minimal noise:

```text
README.md
RELEASE_V0_1_SHOWABLE.md
PROJECT_CANON_AND_STATUS.md
Next.md
ROADMAP.md
```

Do not stuff huge historical notes into `Next.md`.

## README should include

- project name;
- app targets;
- build commands;
- test commands;
- run commands if known;
- map ownership note;
- asset policy note;
- current status;
- known limitations.

## RELEASE_V0_1_SHOWABLE.md should include

- what is included;
- what is not included;
- first route checklist;
- how to validate map anchors;
- how to run smoke checks;
- known limitations;
- next post-v0.1 work.

## Asset note wording

Use wording like:

```text
The project does not require or include copyrighted third-party game assets. Local developer-only asset overrides are optional and are not part of the repository or release package.
```

Do not mention how to rip assets.

## Acceptance

- build instructions match real commands;
- smoke test command works;
- docs do not tell the agent/user to touch the map;
- docs do not require copyrighted assets;
- docs identify `v0.1-showable` limitations clearly.

---

# 21. FILE-BY-FILE GUIDANCE

This section is guidance, not permission to invent files blindly.

Open each file before editing.

## 21.1 `include/StoryRoute.hpp` / `src/StoryRoute.cpp`

Possible work:

- define canonical v0.1 route stages;
- expose route summary;
- prevent route regression;
- map route stage to objective text;
- map route stage to required semantic anchor;
- expose compact route progress for launcher/Pip-Pad.

Do not hardcode map coordinates.

## 21.2 `include/Progression.hpp` / `src/Progression.cpp`

Possible work:

- connect access cards, Pip-Pad, BT-72, combat, service, recovery;
- ensure save/load progression state remains monotonic;
- expose flags as stable names.

## 21.3 `include/GameRuntime.hpp` / `src/GameRuntime.cpp`

Possible work:

- runtime event dispatch for route beats;
- first combat outcome;
- first service/rest;
- recovery node activation;
- Pip-Pad runtime view;
- BT-72 prompt feedback.

## 21.4 `include/GameRuntimePipPad.hpp` / `src/GameRuntimePipPad.cpp`

Possible work:

- route objective text;
- recovery status;
- BT-72 status;
- service prompts;
- incident/merchant window visibility if already present.

## 21.5 `include/WorldValidation.hpp` / `src/WorldValidation.cpp`

Possible work:

- required v0.1 anchor checks;
- missing anchor report;
- severity levels;
- route compatibility check.

Do not edit map files.

## 21.6 `include/WorldSemanticAuthoring.hpp` / `src/WorldSemanticAuthoring.cpp`

Possible work:

- semantic descriptor list;
- anchor metadata;
- author-facing labels/reasons;
- synthetic descriptors for tests.

Do not place anchors in the real map.

## 21.7 `include/HangarSystem.hpp` / `src/HangarSystem.cpp`

Possible work:

- BT-72 restoration stages;
- core install;
- sync/link;
- hangar tutorial completion;
- service status.

## 21.8 `include/LanlineServices.hpp` / `src/LanlineServices.cpp`

Possible work:

- services unlock mirror;
- recovery backbone status;
- selected-world summary;
- merchant window state if existing.

## 21.9 `include/SessionProfiles.hpp` / `src/SessionProfiles.cpp`

Possible work:

- profile/world route state persistence;
- selected world preview;
- atomic save/load consistency;
- no stale global route cache.

## 21.10 `Launcher/src/Launcher_Main.cpp`

Possible work:

- route summary display;
- selected world/profile status;
- recovery handoff summary;
- services unlock preview;
- map validation warnings.

## 21.11 `Editor/src/Editor_Main.cpp`

Possible work:

- display validation report;
- missing v0.1 anchor list;
- export warning.

Forbidden:

- creating map content;
- placing anchors into actual authored map files.

## 21.12 `tests/*`

Possible work:

- add synthetic descriptors;
- route smoke tests;
- save/load smoke tests;
- launcher preview smoke tests;
- BT-72/combat smoke tests;
- recovery handoff smoke tests;
- asset fallback smoke tests.

---

# 22. ROUTE STATE DESIGN NOTES

Use existing code where possible.

If you need to create route states, keep them compact.

Example conceptual enum:

```cpp
enum class FirstRouteStage {
    CryoWake,
    EmergencyMelee,
    AccessCard,
    PipPadAcquired,
    ArchiveTrail,
    EngineeringEntry,
    BT72HullFound,
    BT72CoreFound,
    BT72Restoration,
    BT72Linked,
    HangarTutorial,
    BunkerExitUnlocked,
    SurfaceArrival,
    HeavyClearance,
    FirstCombat,
    FirstServiceRest,
    FirstRecoveryNode,
    Debrief,
    IndustrialHandoff
};
```

Do not add this exact enum if an existing one is present.

## Monotonic route rule

A route state may advance. It should not regress automatically.

If the game needs to show sub-objectives, represent them separately from the main route stage.

Example:

```text
Main stage: SurfaceArrival
Sub objective: Inspect heavy obstruction
```

Do not downgrade main stage to `BunkerExitUnlocked` because the player has not inspected the obstruction yet.

---

# 23. SAVE / LOAD RULES

Save/load must preserve:

- selected world id;
- selected profile id;
- route stage;
- route flags;
- Pip-Pad acquired state;
- access card/clearance state;
- BT-72 restoration state;
- BT-72 sync/link state;
- first combat result;
- first service/rest result;
- first recovery node state;
- debrief state;
- industrial handoff state;
- route event lifecycle state if present;
- service unlock state if present;
- launcher read-state only where appropriate.

Do not mix per-world state and global state accidentally.

If a state belongs to a world, store it world-scoped.

If a state belongs to a profile, store it profile-scoped.

If a state belongs to UI only, do not let it drive gameplay.

---

# 24. LAUNCHER TRUTH MODEL

Launcher is the entry shell, not the gameplay authority.

Launcher may display:

- selected profile;
- selected world;
- route checkpoint;
- next payoff;
- recovery status;
- services unlock status;
- validation warnings;
- announcements/read-state.

Launcher must not invent its own route state.

Correct:

```text
Launcher reads world/profile route snapshot generated by shared route/progression code.
```

Incorrect:

```text
Launcher has separate if/else ladder that guesses story stage from UI-only flags.
```

---

# 25. PIP-PAD TRUTH MODEL

Pip-Pad is in-game player-facing system UI.

It should show:

- current objective;
- route checkpoint;
- BT-72 status;
- service/rest hints;
- recovery backbone status;
- route incidents if unlocked;
- missing requirement hints where appropriate.

It should not show:

- editor-only map authoring instructions to the player;
- debug anchor IDs unless in debug mode;
- unavailable future systems as active objectives;
- stale objectives from earlier route states.

---

# 26. BT-72 DESIGN LOCK

BT-72 is:

- companion platform;
- engineering platform;
- combat platform;
- service/modification loop anchor;
- route progression anchor;
- recovery backbone participant.

BT-72 is not:

- a player-built camp;
- a world editor;
- a normal car;
- a free finished tank at start;
- a shop item;
- a pay-to-win object.

For `v0.1`, make BT-72 readable, not huge.

---

# 27. CAMP / AIMP DESIGN LOCK

Camp/AIMP is:

- portable;
- deployable;
- limited-radius player construction;
- separate from authored world editor;
- separate from workshops;
- separate from BT-72.

For `v0.1`, do not expand Camp/AIMP unless the current code already has a small hook that supports recovery/industry/logistics.

Do not turn Camp/AIMP into a blocker for first vertical slice.

---

# 28. WORKSHOPS DESIGN LOCK

Workshops are authored world nodes.

They are:

- placed by developer/map author;
- found;
- cleared;
- captured;
- restored;
- used for recovery/industry/service loops.

You may implement workshop logic against semantic nodes.

You must not place workshops on the map.

---

# 29. RANDOM / ROUTE EVENT LAYER

If route events already exist, harden them.

Required behavior:

- locked before onboarding/debrief if canon says so;
- offered/active/success/failed/expired/cooldown lifecycle if already present;
- no permanent market on map for one merchant incident;
- state persists;
- Pip-Pad/service glue reads same state.

Do not add a giant random-event framework.

---

# 30. FRIENDLY FIRE / MODES

Friendly fire is allowed only as a mode/setting if already present or trivial:

```text
PvE
PvP
PvW
```

Do not make always-on friendly fire a core base-game blocker.

---

# 31. REACTIVE TECH STACK BOUNDARY

Reactive tech stack is quality layer, not a separate game.

Allowed for `v0.1`:

- honest awareness;
- trigger discipline;
- blocker-aware movement;
- interactables;
- projectile feedback;
- muzzle fire / shock wave;
- limited modular damage;
- limited debris;
- limited foliage/water visual response if already cheap.

Not allowed as base-game blockers:

- full fluid sim;
- total destruction sandbox;
- levolution;
- full fire propagation;
- crowds;
- massive AI rewrite.

---

# 32. ERROR HANDLING AND DIAGNOSTICS

All important systems should fail clearly.

Bad:

```text
return false;
```

Better:

```text
Missing required v0.1 route anchor: bt72_core. BT-72 restoration cannot complete without it.
```

Diagnostics should be available to:

- tests;
- editor validation;
- launcher preview if relevant;
- runtime debug output if relevant.

Player-facing text should be short and diegetic. Developer-facing text can include anchor IDs.

---

# 33. DOCUMENTATION CLEANUP RULES

`PROJECT_CANON_AND_STATUS.md` is canon/status, not active backlog.

`Next.md` is active unfinished work only.

`trash.md` is backlog/later ideas.

Do not dump this entire directive into those files.

At the end of work:

- remove completed active todos from `Next.md`;
- add final `v0.1-showable` status to `ROADMAP.md`;
- add build/run/test instructions to README/release notes;
- keep canon clean.

---

# 34. EXACT FINAL CHECKLIST

Before final response to the human, run only static checks:

```bash
git status --short
git diff --check
git diff --name-only
```

Then provide the human with manual build commands:

```bash
cmake -S . -B build_finish_debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build_finish_debug --parallel
ctest --test-dir build_finish_debug --output-on-failure
```

If using MSVC:

```bash
cmake -S . -B build_finish_msvc -G "Visual Studio 17 2022" -A x64
cmake --build build_finish_msvc --config Debug --parallel
ctest --test-dir build_finish_msvc -C Debug --output-on-failure
```

Then verify by inspection:

```bash
git diff --name-only | grep -Ei '(^|/)(map|maps|world|worlds|level|levels|scene|scenes|terrain|tiles|prefabs|layout|layouts|content|assets)(/|$)|\.(map|world|level|scene|terrain|prefab|tmx|tsx|json)$' || true
```

If this prints authored map/content files, explain and revert unless safe system files.

---

# 35. FINAL RESPONSE FORMAT FOR CLI AGENT

When finished, respond to the human in this format:

```text
Done / Partially done / Blocked

Branch:
<branch>

Human build handoff:
<commands for human to run; do not claim result unless human provided it>

Human tests handoff:
<commands for human to run; do not claim result unless human provided it>

Changed systems:
- ...

Map/content files:
- none changed
OR
- explain exactly why a non-map system file matched the audit pattern

v0.1 route status:
- cryo wake: done/pending
- Pip-Pad: done/pending
- BT-72 restoration: done/pending
- surface arrival: done/pending
- first combat: done/pending
- first service/rest: done/pending
- recovery node: done/pending
- debrief: done/pending
- industrial handoff: done/pending

Known limitations:
- ...

Next human action:
- place/adjust map anchors in the editor according to validation report
```

Do not claim build/test success unless the human confirms build/tests passed.

---

# 36. MINI PROMPT FOR WEAKER CLI MODEL

If the CLI model is weak, paste this at the top of its session before this file:

```text
You are editing an existing C++/CMake game repository. Read PROJECT_FINISH_DIRECTIVE_FOR_CLI_AGENT.md completely. Do not touch the map or authored content. Do not use copyrighted Fallout/Bethesda assets or BA2 extraction. Work in small compile-conscious patches. The human developer builds manually; do not trust or claim your own build/test result. Finish v0.1-showable by hardening route, BT-72, first combat, recovery/industry/logistics, launcher/profile/save-load, validation, and docs. If a system already exists, extend it; do not rewrite. Before final answer, prove changed files, static checks, manual build commands for the human, and that no map files changed.
```

---

# 37. FIRST PATCH SCRIPT FOR CLI AGENT TO RUN MENTALLY

Do not run this as a blind shell script. Use it as a checklist.

```text
1. Inspect git status.
2. Inspect CMake targets.
3. Inspect build target definitions and CMake source lists.
4. Prepare manual build/smoke commands for the human.
5. Search route/BT-72/recovery systems.
6. Search world validation/semantic systems.
7. Identify already-implemented pieces.
8. Add missing tests first where feasible.
9. Implement smallest missing contract.
10. Run static checks only.
11. Verify no map files changed and hand off build/test commands to the human.
12. Commit or leave clean diff summary.
13. Continue next patch.
```

---

# 38. V0.1 ROUTE VALIDATION MATRIX

Use this matrix to align route systems.

| Route stage | Required system | Required anchor? | Player-facing UI | Save/load? | Launcher? | Smoke? |
|---|---|---:|---|---:|---:|---:|
| cryo wake | story/progression | yes | runtime/Pip-Pad | yes | yes | yes |
| emergency melee | combat/progression | yes | runtime/Pip-Pad | yes | optional | yes |
| access card | progression/access | yes | runtime/Pip-Pad | yes | yes | yes |
| Pip-Pad | UI/progression | yes | runtime | yes | yes | yes |
| archive trail | story/progression | yes | Pip-Pad | yes | yes | yes |
| BT-72 hull | hangar/story | yes | runtime/Pip-Pad | yes | yes | yes |
| BT-72 core | hangar/story | yes | runtime/Pip-Pad | yes | yes | yes |
| restoration | hangar/service | yes | runtime/Pip-Pad | yes | yes | yes |
| sync/link | BT-72/profile | yes | runtime/Pip-Pad | yes | yes | yes |
| hangar tutorial | runtime | yes | runtime/Pip-Pad | yes | yes | yes |
| bunker exit | story/world state | yes | runtime/Pip-Pad | yes | yes | yes |
| surface arrival | story/runtime | yes | runtime/Pip-Pad | yes | yes | yes |
| heavy clearance | BT-72/world state | yes | runtime/Pip-Pad | yes | yes | yes |
| first combat | combat/BT-72 | yes | runtime/HUD/Pip-Pad | yes | yes | yes |
| service/rest | service/BT-72 | yes | runtime/Pip-Pad | yes | yes | yes |
| recovery node | recovery/world | yes | runtime/Pip-Pad | yes | yes | yes |
| debrief | story/recovery | yes | runtime/Pip-Pad | yes | yes | yes |
| industrial handoff | industry/logistics | yes | Pip-Pad/Launcher | yes | yes | yes |

Do not place these anchors in the map. Only validate them.

---

# 39. COPY BLOCK — REQUIRED ANCHOR SPEC TEXT

Use this exact semantic intent if creating docs or validation labels.

```text
cryo_start: first playable wake point for the bunker start.
emergency_melee_pickup: early survival tool before firearms/BT-72.
first_vermin_gate: first low-risk hostile pressure point.
bunker_access_card: early non-Pip-Pad clearance gate.
pip_pad_pickup: acquisition point for the primary system interface.
archive_trail_start: start of paper/archive/data breadcrumb path.
archive_trail_end: archive trail handoff into engineering layer.
engineering_entry: route transition into BT-72 discovery layer.
bt72_hull: authored discovery point for the BT-72 hull.
bt72_core: authored discovery point for the BT-72 core.
bt72_service_notes: service/repair hint source for staged restoration.
bt72_material_cache: early materials source for restoration.
bt72_restoration_bay: place where staged restoration is completed.
bt72_sync_station: sync/link activation point.
hangar_tutorial: controlled tutorial beat for BT-72 movement/role.
bunker_exit_gate: exit unlock / hangar gate / lift route semantic point.
surface_arrival: first confirmed surface checkpoint.
heavy_clearance_blocker: first BT-72 heavy clearance payoff point.
first_combat_zone: first combat encounter semantic zone.
first_service_bay: first service/rest handoff point after combat.
first_recovery_node: first recovery payoff node.
debrief_terminal: debrief/handoff point.
industrial_handoff_node: transition into recovery/industry/logistics backbone.
```

---

# 40. COPY BLOCK — RELEASE NOTES SKELETON

Create `RELEASE_V0_1_SHOWABLE.md` or adapt existing docs.

```markdown
# Bunker Protocol — v0.1 Showable

## Included

- Launcher-first entry flow.
- First playable route state from cryo wake to industrial handoff.
- Pip-Pad objective/status layer.
- BT-72 discovery, staged restoration, sync/link, first combat role.
- First service/rest handoff.
- First recovery node and debrief into industry/logistics backbone.
- Save/load persistence for route/profile/world state.
- Editor/world validation for required v0.1 semantic anchors.
- Smoke checks for route, BT-72, recovery, launcher/profile glue.

## Not included

- Final authored map/layout.
- Player-side world editor.
- Full open-world city.
- DLC/expansion content.
- MMO/backend services.
- Copyrighted third-party game assets.

## Build

These commands are for the human developer to run manually. The CLI agent must not claim they passed unless the human confirms it.

```bash
cmake -S . -B build_finish_debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build_finish_debug --parallel
ctest --test-dir build_finish_debug --output-on-failure
```

## Map authoring handoff

The map is authored separately by the developer. The code expects semantic anchors and reports missing anchors through validation. The finishing pass does not place or modify map content.

## Asset policy

The project does not require or include copyrighted third-party game assets. Local developer-only asset overrides are optional and are not part of the repository or release package.

## Known limitations

- Visual content may use placeholders.
- Final map layout is developer-authored outside this pass.
- Mid-game industry/logistics is a backbone handoff, not the full final game.
```

---

# 41. COPY BLOCK — README BUILD SECTION

Use only if README lacks build/run instructions.

```markdown
## Build and smoke test

Run these manually from the repository root.

```bash
cmake -S . -B build_finish_debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build_finish_debug --parallel
ctest --test-dir build_finish_debug --output-on-failure
```

On Visual Studio/MSVC:

```bash
cmake -S . -B build_finish_msvc -G "Visual Studio 17 2022" -A x64
cmake --build build_finish_msvc --config Debug --parallel
ctest --test-dir build_finish_msvc -C Debug --output-on-failure
```

## Targets

- `BunkerLauncher` — launcher/profile/world shell.
- `BunkerGame` — runtime gameplay.
- `BunkerEditor` — developer-facing authored-world tool.
- `BunkerSmokeChecks` — smoke coverage for base route and glue.

## Map boundary

The final map is authored manually by the developer. Game systems use semantic anchors and validation reports. Automated finishing work must not edit authored map content.

## Asset boundary

The repository does not require or include copyrighted third-party game assets. Use original, licensed, placeholder, or local developer-provided assets only.
```

---

# 42. COPY BLOCK — SMOKE TEST PSEUDOCODE

Use current test framework and names. Do not paste blindly if incompatible.

```cpp
TEST(FirstRouteValidation, CompleteV01DescriptorPasses) {
    auto descriptor = MakeSyntheticV01WorldDescriptorWithAllRequiredAnchors();
    auto result = ValidateV01RouteAnchors(descriptor);
    EXPECT_TRUE(result.ok);
    EXPECT_TRUE(result.missingRequiredAnchors.empty());
}

TEST(FirstRouteValidation, MissingBT72CoreFails) {
    auto descriptor = MakeSyntheticV01WorldDescriptorWithAllRequiredAnchors();
    descriptor.RemoveAnchor("bt72_core");
    auto result = ValidateV01RouteAnchors(descriptor);
    EXPECT_FALSE(result.ok);
    EXPECT_THAT(result.missingRequiredAnchors, Contains("bt72_core"));
}

TEST(FirstRouteValidation, MissingRecoveryNodeFails) {
    auto descriptor = MakeSyntheticV01WorldDescriptorWithAllRequiredAnchors();
    descriptor.RemoveAnchor("first_recovery_node");
    auto result = ValidateV01RouteAnchors(descriptor);
    EXPECT_FALSE(result.ok);
    EXPECT_THAT(result.missingRequiredAnchors, Contains("first_recovery_node"));
}
```

Adapt to the repo's actual test framework. If no GoogleTest is used, use the existing smoke-check style.

---

# 43. COPY BLOCK — ROUTE OBJECTIVE TEXT

Use these strings or adapt tone.

```text
CryoWake: Stabilize after cryo wake.
EmergencyMelee: Find an emergency weapon.
AccessCard: Recover a valid access card.
PipPadAcquired: Bring the Pip-Pad interface online.
ArchiveTrail: Follow the archive trail into engineering.
BT72HullFound: Inspect the BT-72 hull.
BT72CoreFound: Recover the BT-72 core.
BT72Restoration: Restore the BT-72 systems in stages.
BT72Linked: Complete BT-72 sync/link.
HangarTutorial: Test BT-72 control inside the hangar.
BunkerExitUnlocked: Open the bunker exit route.
SurfaceArrival: Establish first surface contact.
HeavyClearance: Use BT-72 force to clear the obstruction.
FirstCombat: Survive the first hostile contact.
FirstServiceRest: Service the BT-72 and recover.
FirstRecoveryNode: Secure the first recovery node.
Debrief: Return and debrief.
IndustrialHandoff: Bring the recovery/industry/logistics backbone online.
```

---

# 44. COPY BLOCK — ASSET FALLBACK BEHAVIOR

Use this behavior, not necessarily this code.

```text
When a material/texture is missing:
1. record a diagnostic with requested asset id/path;
2. return a neutral placeholder material;
3. continue runtime if the asset is non-critical;
4. fail validation only if the asset is marked required for shipping;
5. never require third-party game archives.
```

Potential placeholder names:

```text
placeholder/default_albedo
placeholder/default_normal
placeholder/missing_material
placeholder/debug_checker
```

Do not use third-party commercial game names.

---

# 45. COPY BLOCK — FINAL HUMAN SUMMARY TEMPLATE

```markdown
# Finish pass summary

## Status

Partially done / Done / Blocked

## Human build handoff

- Command for human:
- Human-reported result:

## Human tests handoff

- Command for human:
- Human-reported result:

## Changed

- Route:
- BT-72:
- Combat:
- Recovery/industry/logistics:
- Launcher/profile/save-load:
- Editor validation:
- Docs:

## Map boundary

No authored map/content files were changed.

## Asset boundary

No copyrighted third-party asset extraction or dependency was added.

## Remaining human work

- Author map layout.
- Place required semantic anchors.
- Replace placeholders with original/licensed assets.
```

---

# 46. FINAL WARNING TO CLI AGENT

Do not be clever.

Do not restart the project.

Do not touch the map.

Do not use Fallout/Bethesda assets.

Do not turn the task into a giant engine rewrite.

Patch, statically verify, hand off build/test commands to the human, repeat after the human reports results.

The finish line is `v0.1-showable`, not perfection.

