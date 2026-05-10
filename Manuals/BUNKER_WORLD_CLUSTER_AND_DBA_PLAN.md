# Bunker World Cluster and DBA Plan

This note captures the v9.26 architecture direction without changing BWL7.

## Current rule

- `.bwld` remains the canonical authored world/source-of-truth file.
- BWL7 stays intact in this pass.
- Runtime/profile/static eraser data remains separate from authoring worlds.

## Cluster direction

A large seamless authored world can later be split into multiple `.bwld` cluster files.

Planned cluster concerns:

- `clusterId`
- `worldspaceId`
- `worldspaceName`
- grid or bounds
- origin offset
- neighbor cluster ids
- streaming hints/radius
- dependency package ids
- local objects
- local semantic anchors
- local loot/link data
- local nav/interaction metadata

This is a planning/documentation layer only in v9.26. No BWL8 bump is required for the current registry/editor work.

## `.dba`

`.dba` means `Data Bunker Archive`.

It is planned as the Bunker-native DLC/mod/package format and is **not** a Bethesda `.ba2`/`.bsa`.

Intended package contents later:

- one or more `.bwld` clusters
- Bunker-native assets
- manifests
- dependency/load-order metadata
- localization
- behavior/script data
- LAN-safe package descriptors
- hashes/signatures

## Safety rules

- `.dba` must not silently overwrite base authored worlds.
- `.dba` must not be treated as a raw Bethesda archive.
- user-owned external Fallout/Bethesda files remain reference-only/staging data until dedicated converters exist.
- solo mode must not depend on hidden network services.
- LAN validation should later compare stable package ids, hashes, and load order.

## Follow-up

- If cluster metadata ever needs binary world changes, plan that as BWL8 separately.
- BWL8 must keep loading BWL7 worlds.
