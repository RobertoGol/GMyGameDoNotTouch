# Branch Canon Reconciliation Notes

This note records canon/document differences discovered during the v9.26 branch audit. No source document was deleted or overwritten here.

## Root MD -> Manuals move

On `Testing-imgui-viewports`, the former root canon/planning files are present under `Manuals/`:

- `Manuals/AllIn.MD`
- `Manuals/Next.md`
- `Manuals/Next_compact.md`
- `Manuals/PROJECT_CANON_AND_STATUS.md`
- `Manuals/PROJECT_FINISH_DIRECTIVE_FOR_CLI_AGENT.md`
- `Manuals/ROADMAP.md`
- `Manuals/Test.md`
- `Manuals/trash.md`

## Compared against `master`

### `PROJECT_CANON_AND_STATUS.md`
- `master`: root file
- `Testing-imgui-viewports`: `Manuals/PROJECT_CANON_AND_STATUS.md`
- Audit result: content differs, not just location.
- Recommendation: keep both histories intact, treat current code/tests as primary truth, and reconcile wording deliberately in a later docs-only pass.

### `PROJECT_FINISH_DIRECTIVE_FOR_CLI_AGENT.md`
- `master`: root file
- `Testing-imgui-viewports`: `Manuals/PROJECT_FINISH_DIRECTIVE_FOR_CLI_AGENT.md`
- Audit result: content differs, not just location.
- Recommendation: preserve both; use current branch version for current workflow, but review master deltas before any future “single canon doc” cleanup.

### `ROADMAP.md`
- Audit result: moved copy matches `master` in content.
- Recommendation: treat as pure location move to `Manuals/`.

### `Next.md`
- Audit result: moved copy matches `master` in content.
- Recommendation: treat as pure location move to `Manuals/`.

### `Next_compact.md`
- Audit result: moved copy matches `master` in content.
- Recommendation: treat as pure location move to `Manuals/`.

### `trash.md`
- Audit result: diff exists in git comparison output, but sampled content appears functionally the same apart from formatting/line-ending style.
- Recommendation: do not auto-delete either copy; if needed, normalize formatting in a docs-only cleanup pass.

## Guidance

- Do not delete either side of a differing canon/planning document without explicit user approval.
- For implementation conflicts, trust current code and smoke checks first.
- For project intent conflicts, compare `Manuals/PROJECT_CANON_AND_STATUS.md`, `Manuals/Next.md`, and branch-specific archive notes before changing code.
