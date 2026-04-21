# Next Split

`Next.md` был разложен на более короткие файлы по целевым местам.

Главное:

- общий план: `Next_split/00_MASTER_PLAN.md`
- launcher: `Next_split/Launcher/src/Launcher_Main.cpp.md`
- services/persistence:
  - `Next_split/include/SessionProfiles.hpp.md`
  - `Next_split/include/LanlineServices.hpp.md`
  - `Next_split/src/LanlineServices.cpp.md`
- runtime/world:
  - `Next_split/include/GameRuntime.hpp.md`
  - `Next_split/src/GameRuntime.cpp.md`
  - `Next_split/include/World.hpp.md`
  - `Next_split/src/World.cpp.md`
- progression/story:
  - `Next_split/include/Progression.hpp.md`
  - `Next_split/src/Progression.cpp.md`
  - `Next_split/include/StoryRoute.hpp.md`
  - `Next_split/src/StoryRoute.cpp.md`
- tank/editor:
  - `Next_split/include/HangarSystem.hpp.md`
  - `Next_split/Editor/src/Editor_Main.cpp.md`
- docs:
  - `Next_split/ROADMAP.tasks.md`
  - `Next_split/Use_this_One.tasks.md`
  - `Next_split/trash.tasks.md`

Это не новый источник истины по проекту, а разбор содержимого старого `Next.md`, чтобы им было проще пользоваться и быстрее дочищать хвосты.

Текущий активный пакет после последнего editor-hardening прохода:

- `Next_split/Editor/src/Editor_Main.cpp.md` for `prefab/library` + `import assistant`
- затем `Next_split/include/LanlineServices.hpp.md` / `Next_split/src/LanlineServices.cpp.md`
- затем launcher/runtime хвосты по соответствующим split-файлам
