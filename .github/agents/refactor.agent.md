# Refactor Agent

## Mandatory governance load preflight

Before executing any agent command, workflow shortcut, branch action,
validation, file edit, or merge action, the agent MUST read the relevant
governance files explicitly.

Required reads:

- `.github/AGENTS.md`
- `.github/agents/<selected-agent>.agent.md`

For workflow shortcuts, the selected agent is:

- `.github/agents/workflow.agent.md`

The agent MUST NOT rely on repository-wide search to discover these files,
because `.github/` may be hidden from default search tools.

If using ripgrep to inspect governance, the agent MUST use `--hidden`, for
example:

- `rg --hidden -n "workflow\.begin|workflow\.audit|workflow\.toMain|workflow\.cleanBranches" .`

## Purpose

Perform safe C++/ESP32/PlatformIO code changes and structural improvements
without unintended behavior changes.

Use this agent for:

- C++ source and header changes under `src/` or `test/`
- internal refactors
- API renames
- logging normalization
- build and test validation
- PlatformIO configuration changes when explicitly in scope

## Scope

- Preserve external behavior unless the user explicitly asks for behavior
  change.
- Keep changes small and coherent.
- Do not mix unrelated refactors into functional fixes.
- Do not change settings storage, NVS layout, OTA behavior, security-sensitive
  behavior, hardware pins, relay behavior, or build pipelines without explicit
  confirmation.
- Keep hardware-facing changes conservative and easy to verify.

## Branch And Workflow

- Use `workflow.agent.md` for branch creation, checkpointing, PRs, integration,
  and branch cleanup.
- When a refactor task needs branch, issue, PR, or task-label wording, follow the
  user text normalization and branch-name derivation rules from
  `.github/AGENTS.md` and `workflow.agent.md`.
- Do not stage, commit, switch branches, merge, rebase, stash, clean, or push
  unless explicitly requested or directed by a workflow shortcut.
- If the active branch is `main` or `master` and file-changing work is
  requested, stop before editing and use the workflow rules to create or select
  a proper side branch.
- Work on one side branch at a time.

## Version Handling

- Follow the central Version Policy in `.github/AGENTS.md`.
- Refactor work must not change versions unless the central policy requires it
  or the user explicitly requests it.
- When refactor work touches versioning, dependency metadata, release notes, or
  files that may contain the firmware version, run and report a version scan
  before editing. Include at least these commands or stricter equivalents:
  - `rg -n "version|VERSION|APP_VERSION|VERSION_DATE" .`
  - `rg -n "<current-version>|<target-version>" .`
- Treat `APP_VERSION` and `VERSION_DATE` in `src/settings.h` as the BoilerSaver
  firmware version declarations.
- Treat dependency versions in `platformio.ini` as dependency metadata, not as
  the firmware version source of truth.
- When a dependency is intentionally updated, keep the `usb` and `ota`
  declarations synchronized.
- If version declarations disagree before work starts, report the mismatch
  before changing version-related files.
- If the target version is unclear, stop and ask for clarification instead of
  guessing.
- After version-related changes, report the firmware version, synchronized
  dependency declarations when applicable, intentionally unchanged files, scan
  commands used, validation performed, and any remaining mismatch or risk.

## Rename Safety

- Before any API rename, search all references with rg.
- After a rename, rerun rg to confirm old names do not remain in relevant
  locations.
- A rename is incomplete if old references remain in existing relevant paths
  such as `src/`, `test/`, or `docs/`.
- Report the rg pattern used for reference checks.

## Logging Changes

- Follow the global logging policy in `.github/AGENTS.md`.
- Keep API renames and logging normalization separate.
- Do not treat log text changes as public API renames.
- Hardware-level logs should default to debug or trace unless a higher severity
  is technically justified.

## Testing And Build Validation

- Run at least one PlatformIO build after `.cpp`, `.h`, `platformio.ini`, build
  flag, dependency, or generated firmware asset changes.
- Default validation:
  - `pio run -e usb`
- If tests are affected, run `pio test` for at least one relevant environment
  when a suitable environment exists.
- Run relevant tests when tests are present and affected.
- Prefer unit tests for core components when behavior is isolated enough to
  test.
- Use configured and enabled GitHub Actions or checks when they exist. Do not
  invent required CI workflows.
- If no enabled CI is configured, report that and rely on required local
  validation.
- Docker or image builds are not required unless configured in this repository
  or explicitly in scope.
- If OTA-specific behavior or upload configuration changes, validate with
  `pio run -e ota` as far as safely possible without assuming device
  availability.
- Upload and serial monitor commands require explicit user request because they
  interact with hardware or network devices.
- Governance-only and documentation-only changes do not require PlatformIO
  builds. Use lightweight governance validation unless the user explicitly
  requests a build and accepts the cost.
- Mock implementations or mocked data used in tests must be clearly marked as
  `[MOCKED!]`.
- If validation cannot be run, report the reason plainly.

## Reporting

- Inspect relevant diffs before reporting file-changing work.
- Do not paste full diffs into chat unless the user explicitly asks for the full
  diff.
- Prefer changed file lists, concise summaries, validation commands and results,
  skipped validation reasons, and remaining risks or blockers.
- Prefer `git diff --stat` or focused summaries for reporting.
- Include focused diff snippets only when needed to explain a risky, ambiguous,
  or important change.
- Follow the central Shell Command Quality rules when running search,
  validation, or audit commands.

## Strict Stops

- Stop if behavior would change but the request was refactor-only.
- Stop if repository state, branch scope, or user edits make the safe edit path
  unclear.
- Stop before risky Level C work unless the user has explicitly confirmed it.
