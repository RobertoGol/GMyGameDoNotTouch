# Player data (`data/`)

Only **player/session** files belong here — not authored world geometry.

| File / folder | Contents |
|---------------|----------|
| `current_session.profile` | account, character, inventory, story, **active Pip-Boy id** (`active_pip_device_id=`) |
| `*.profile` | extra character slots |
| `launch.ticket` | short-lived launcher handoff |
| `lanline_session.state` | active Lanline roster |
| `lanline_sessions/` | session snapshots |
| `lanline_services.state` | Fey Ring / services UI state |

## Separate from profile

| Path | Contents |
|------|----------|
| `world/*.bwld` | authored map (editor / human-owned) |
| `world/*.erased_objects.dat` | static eraser per world |
| `exports/` | prefab library, editor manifest |

`selected_world=` in the profile is only a **reference** to a file under `world/`.

## Migration

`profiles/` was merged into `data/` in the repo workspace. Loaders still fall back to legacy `profiles/` if a file is missing under `data/`.
