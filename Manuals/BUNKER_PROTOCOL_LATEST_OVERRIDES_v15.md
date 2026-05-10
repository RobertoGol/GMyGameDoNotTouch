# BUNKER_PROTOCOL_LATEST_OVERRIDES_v15.md

This is the compact override file for the huge canon/mechanics bible.  
Use this first when an old section conflicts with newer decisions.

## Priority

```text
Latest override wins.
Current code and smoke-checks still win for implementation.
Old repeated sections are historical context, not final truth.
```

## Old → New override table

| Старый / устаревший вариант | Новый / правильный override |
|---|---|
| `Pip-Pad работает с самого пробуждения` | Игрок просыпается **без Pip-Pad**. Сначала табло, терминалы, свет, signage, access/clearance. Pip-Pad надо найти. |
| `Pip-Boy 3000` и `Pip-Boy 3000 Mark IV` как разные модели | **Pip-Boy 3000 = Pip-Boy 3000 Mark IV**. Это один accepted/selectable model. |
| `Pip-Boy 3000 Mark V` как обычный вариант | **Mark V / TV-style** — rejected heavy failed model, not selectable, trash/scrap prop. |
| `TV-style Mark V без MAP как факт` | В референсе MAP mode может быть, но в Bunker Protocol Mark V — prop/demo only and grants no gameplay map. |
| `Continuity Anchor временное имя` | **Continuity Anchor / Якорь Непрерывности** — preferred player/lore name. |
| `SoulLine / Линейка Сознания главное имя` | Оставить как **legacy/internal alias** для Continuity Anchor. |
| `Pip-Pad = обычный планшет` | Pip-Pad — **rugged military/corporate field tablet evolved from Pip-Boy 3000 / Mark IV logic**. |
| `Pip-Pad сам хранит всю диагностику/логистику` | Pip-Pad читает/индексирует/переводит данные из media, terminals, BT-72 sync, recovery/relay data. Не магический источник. |
| `BlueLink = обычный Bluetooth-плеер` | BlueLink — **field media expansion module** для Pip-Pad expansion bay, не consumer player. |
| `Pip-Pad expansion cover = модуль` | Заглушка только защищает порт. Media unlock появляется после установки BlueLink. |
| `FO76-style Pip-Boy — плохой/вторичный вариант` | Это user-preferred comfortable shell. Делать его пригодным и уважительно, даже с ограничениями. |
| `Pip-Boy 1.0 цифровой как остальные` | Pip-Boy 1.0 paper-heavy: maps/checklists/service/logistics mostly on paper/cards. |
| `BT-72 просто hull + core` | Добавить crane chain: hull -> crane -> tank service lift/restoration cradle -> core -> staged restoration -> sync/link. |
| `Bunker 17 вообще без японского слоя` | Визуально cold military/corporate, но можно subtle Japanese/Yamato records, translated notes, names, logs. |
| `Внутри Bunker 17 heavy Japanese visuals` | Нет: torii/fantasy shops/guild houses/Akiba street visuals mostly after surface exit. |
| `Все девайсы имеют одинаковые вкладки` | Device UI capability-driven, tree-style allowed, tabs/subtabs differ by model. |
| `MAP tab всегда есть` | MAP зависит от device capability + route/data unlocks. Some devices use physical navigation. |
| `BunkerGame 2D/grid final` | Final target is 3D immersive runtime; grid/2D shell is prototype/debug only. |
| `Agent can edit map/layout freely` | Authored map geometry is human-owned. Agent uses validation/semantic requirements unless directly told. |

## Device model table

| Model | Status | Map/navigation |
|---|---|---|
| Pip-Boy 0.1 | selectable edge-case / joke fallback | no digital MAP; physical route card only |
| Pip-Boy 1.0 | optional old/paper-heavy shell | limited/local only if authored; mostly paper |
| Pip-Boy 2000 Classic | optional | Automap-style local/discovered maps |
| Pip-Boy 2000 Mark VI / FO76-style | optional, user-preferred comfortable shell | physical navigation behavior; compass/markers/logs allowed |
| Pip-Boy 3000 / Mark IV | accepted/selectable normal Pip-Boy 3000 | digital MAP, local/world/route after unlocks |
| Pip-Boy 3000 Mark V / TV-style | not selectable | prop/demo only; no gameplay map access |
| Pip-Pad 3500 | accepted rugged field tablet | best display/media/map workflow, data still requires sources |

## Pip-Pad + BlueLink current rule

```text
Pip-Pad 3500 has a sealed expansion bay with a dummy cover.
BlueLink Media Module installs into that bay.
BlueLink unlocks media playback, indexing, transcript and translation workflows.
Pip-Pad/BlueLink reads media; it does not invent diagnostics/logistics/map data.
```

## Media sources

```text
holotapes
voice recordings
service tapes
diagnostic tapes
data tapes
archive cartridges
data cards
paper notes
paper checklists
printed maps
terminal downloads
BT-72 diagnostic dump after sync
recovery node reports
relay packets
```

## BT-72 crane restoration current rule

```text
bt72_hull_found
hangar_power_restored
crane_control_online
crane_path_clear
hull_attached_to_crane
hull_moved_to_service_lift
hull_locked_in_restoration_cradle
companion_core_installed
service_notes_checked
bt72_staged_restoration_complete
bt72_sync_linked
tank_tutorial_complete
hangar_exit_gate_unlocked
```

## Recommended new smoke-checks

```text
pippad_has_expansion_cover_before_bluelink
pippad_media_index_locked_without_bluelink
bluelink_install_unlocks_media_index
bluelink_can_read_service_tape
bluelink_can_show_translated_transcript
bt72_crane_requires_hangar_power
bt72_hull_moved_to_service_lift_before_core_install
continuity_anchor_aliases_soulline_legacy_field
pipboy_3000_equals_mark_iv_capability
pipboy_3000_mark_v_not_selectable
fo76_pipboy_shell_is_viable_user_preferred_option
pippad_is_not_consumer_tablet
pippad_bluelink_is_not_consumer_bluetooth_player
japanese_source_media_localizes_to_player_language
```

## Short final wording

```text
Pip-Pad 3500 is a rugged military Pip-Boy-derived field tablet with a covered expansion bay.
BlueLink is the field media module for recordings/data media.
BT-72 restoration requires crane transfer to a tank service lift/restoration cradle.
Continuity Anchor is the preferred name; SoulLine remains legacy/internal alias.
Pip-Boy 3000 means Mark IV; Mark V is rejected prop.
FO76-style Pip-Boy remains a valid user-preferred shell.
```
