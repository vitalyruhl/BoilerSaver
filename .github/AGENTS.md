# AGENTS.md

Repository guidance for contributors and coding agents.

This repository is `BoilerSaver`, a C++/ESP32 firmware project built with
PlatformIO and the Arduino framework.

## Mandatory Governance Load Preflight

Before executing any agent command, workflow shortcut, branch action,
validation, file edit, commit, push, pull request, merge, or cleanup action, the
agent MUST explicitly read the applicable governance files.

Required reads:

- `.github/AGENTS.md`
- `.github/agents/<selected-agent>.agent.md`

For workflow shortcuts, the selected agent is:

- `.github/agents/workflow.agent.md`

The agent MUST NOT rely on repository-wide search to discover these files,
because hidden directories such as `.github/` may be skipped by default search
tools.

If governance files are known by path, read them directly instead of discovering
them through search.

If using ripgrep to inspect governance, the agent MUST include hidden paths, for
example:

`rg --hidden -n "workflow\.begin|workflow\.audit|workflow\.toMain|workflow\.cleanBranches" .`

If using fd to inspect governance, the agent MUST include hidden paths, for
example:

`fd --hidden "AGENTS|agent\.md|workflow\.agent\.md" .`

Plain `rg ... .` or plain `fd ... .` MUST NOT be treated as sufficient for
governance discovery, because they may skip hidden directories such as
`.github/`.

Failure to read the required governance files is a hard blocker and must be
reported before any further action.

## Communication

- Use informal German ("du") when talking to the user.
- Keep explanations short unless detail is explicitly requested.
- After completing work, summarize briefly.
- Internal chat with the user may remain informal German according to these
  communication rules.
- Only normal chat with the user may be German by default. Repository artifacts
  must remain in English unless the user explicitly requests a different
  language for a specific artifact.

## Repository Text Language

- Governance files under `.github/`, repository documentation, generated
  project text, issue text, pull request text, review comments created by
  agents, code comments, error messages, log messages, and commit messages
  created by agents must be written in English.
- Do not translate repository artifacts to German just because the user chat is
  German.
- If a specific artifact must be written in another language, require an
  explicit user request for that artifact.

## User Text Normalization

- User prompts may contain informal German, mixed German/English wording, and
  obvious spelling mistakes.
- Normalize free text into clean English when deriving branch names, issue/PR
  titles, task labels, or governance wording.
- Do not silently normalize exact paths, commands, symbols, identifiers,
  explicitly provided names, issue/PR numbers, tags, versions, or quoted
  literals.
- If the user says to use a name exactly, preserve it.
- Check quoted path casing against repository state.
- If intent is unclear or normalization changes scope, stop and ask.
- Example: `gevernance-sharpenes` may derive `governance-sharpening`.

## GitHub Text Language

- GitHub Issue titles and bodies must be written in English.
- GitHub Pull Request titles and descriptions must be written in English.
- GitHub comments created by agents should be written in English unless the user
  explicitly requests otherwise.
- Do not create German GitHub issue or PR text by default.
- These rules apply only to GitHub and repository artifacts, not to normal chat
  with the user.

## Repository Scope

- Primary project configuration: `platformio.ini`.
- Main firmware source code lives under `src/`.
- Firmware version declarations live in `src/settings.h`.
- Supporting documentation lives under `docs/` and `readme.md`.
- Tests live under `test/` when present.
- Support scripts and repository tooling live under `tools/`.
- Wokwi simulation files live under `Wokwi/`.
- Shared Serena configuration is `.serena/project.yml`.
- Local Serena state under `.serena/project.local.yml`, `.serena/cache/`, and
  `.serena/memories/` must remain ignored.
- Do not assume Python application release flows or a standalone backend web
  service unless the repository explicitly introduces them.

## Agent Routing

- `.github/AGENTS.md` is the canonical source for repository-wide rules.
- `.github/agents/control-plane.agent.md` only routes work to the correct agent.
- Use `.github/agents/refactor.agent.md` for code changes, refactors, tests, and
  build validation.
- Use `.github/agents/workflow.agent.md` for branches, issues, PRs, checkpoints,
  integration, cleanup, and explicit session-close workflows.
- Use `.github/agents/docs.agent.md` for documentation and governance work.
- Agent files may add scope-specific rules, but they must not contradict this
  file. If they do, follow this file and report the drift.

## Tracking Policy

- GitHub Issues may be used for tracked work when useful.
- GitHub Pull Requests may be used for review and integration when useful.
- Tracking is optional unless the user explicitly requests it.
- Do not invent mandatory external tracking updates for ordinary repository
  work.

## Safety Principles

- User changes are sacred. Never revert or overwrite user edits without asking.
- Analyze before modifying files.
- Do not make functional project-code changes unless explicitly requested or
  strictly required to correct governance references.
- Do not invent project rules. Preserve existing project-specific rules unless
  they are clearly obsolete or consciously replaced by newer governance.
- Do not mark an issue as solved or fixed until the user confirms it works.
- Before rename/delete operations, search references first and update them.
- Before terminal commands that modify the workspace, ensure goal and scope are
  clear.
- If requirements are ambiguous or a change impacts multiple subsystems/files,
  ask 1-3 precise questions or propose 2-3 options before editing.
- Work incrementally in small steps: fix, verify, checkpoint or commit only when
  requested, then continue.
- Work on one side branch at a time.
- Before multi-file refactors or other risky changes, ensure the baseline is
  understood and either clean, committed, or intentionally dirty by user
  request.
- Never expose values from `src/secret/secrets.h`. Only
  `src/secret/secrets.example.h` may be tracked.
- Do not modify generated build output under `.pio/`.

## Autonomy Levels

- Level A, safe: read-only actions such as search, read, and analysis may be done
  immediately.
- Level B, normal: small, clearly scoped changes may be implemented immediately.
- Level C, risky: changes involving settings structure, storage/NVS, OTA,
  security, hardware pins, relay behavior, build pipelines, or large refactors
  require explicit confirmation.

## Repository Workflow Rules

- Route branch, PR, merge, checkpoint, and cleanup operations through
  `.github/agents/workflow.agent.md`.
- Integrate into `main` through pull requests by default.
- `main` is the published branch.
- `feature/*` and `fix/*` branches are work branches and may be unfinished.
- Do not change `main` directly.
- If the active branch is `main` or `master` and file-changing work is
  requested, stop before editing and route through
  `.github/agents/workflow.agent.md` to create or select a proper side branch.
- Direct pushes to `main` are forbidden unless the user explicitly requests an
  exception.
- Fast-forward integration to `main` is allowed only when the user explicitly
  requests fast-forward or `ff`.
- Workflow shortcuts may be chained only in sequence. `workflow.audit` remains
  strictly read-only and must finish by reporting blockers before any follow-up
  shortcut runs.
- If `workflow.audit` is followed by `workflow.toMain` or
  `workflow.cleanBranches`, continue only when the user explicitly requested the
  follow-up shortcut and no blockers remain.
- If blockers remain after audit, stop before merge, cleanup, or other
  destructive actions.
- Before preparing or merging changes into `main`, perform a documentation
  impact check.
- The documentation impact check must decide whether changed files or behavior
  require documentation updates.
- If documentation is affected, route the work through
  `.github/agents/docs.agent.md` before integration.
- If a changelog exists and the change is user-visible, dependency-related,
  build-related, or version-related, update it or explicitly justify why no
  changelog update is needed.
- If documentation is not affected, report that documentation sync is not
  required and why.
- Do not invent documentation updates for purely internal or governance-only
  changes unless governance documentation itself changed.
- Governance-only changes do not require changelog entries unless this
  repository intentionally starts tracking governance changes in a changelog.
- Documentation-only changes do not require a version bump.
- Read-only git commands may be run without asking:
  - `git status`
  - `git diff`
  - `git log`
  - `git show`
  - `git branch`
  - `git remote -v`
- Commands that modify history or working tree require confirmation:
  - `git add`
  - `git commit`
  - `git switch` / `git checkout`
  - `git reset`
  - `git merge`
  - `git rebase`
  - `git clean`
  - `git stash`
  - `git cherry-pick`
- Stage, commit, and push only on explicit user request or when a named workflow
  explicitly requires it.
- If staging is requested, prefer `git add -A`.
- Do not prepend `Set-Location` to git commands. Use the configured working
  directory. For non-git commands that must change directory, use
  `Push-Location` / `Pop-Location`.
- Prefer gh for PRs, CI checks, issues, and GitHub operations when available.
- For `workflow.toMain`, commit, push, pull request creation, pull request merge,
  and branch cleanup are allowed only when explicitly requested by the shortcut
  and only after repository state, validation needs, and documentation impact
  have been checked.
- Before merging to `main`, run or report relevant validation, perform the
  documentation impact check, and report GitHub blockers such as required
  reviews, failing checks, conflicts, or branch protection.
- Owner/admin bypass may be used only when the user explicitly requested it for
  the current action. Required status checks must not be bypassed unless the user
  explicitly confirms that exception and the reason is reported.
- For `workflow.cleanBranches`, delete only branches verified as integrated.
  Do not delete active, unmerged, or ambiguous branches, and report skipped
  branches with the reason.

## Pull Request Review Policy

- Maintainer-owned PRs do not require external review by repository governance.
- Maintainer-owned PRs may be merged when required validation passed and GitHub
  branch protection allows the merge.
- External contributor PRs require review before merge. Do not merge unreviewed
  external contributions.
- Branch protection or ruleset bypass may only be used when the user explicitly
  requests owner/admin bypass for the current action.
- Do not use owner/admin bypass automatically.
- If GitHub branch protection blocks a merge because review is required, report
  the blocker and do not bypass branch protection.
- If a maintainer-owned PR is blocked only by a review requirement and required
  validation passed, report that owner/admin bypass may be possible when
  configured.
- If review is not intended for maintainer-owned PRs, recommend adjusting branch
  protection instead of bypassing it.
- Do not bypass external contributor PR review unless the user explicitly
  confirms a repository-owner exception after reviewing the PR.
- Required status checks must not be bypassed unless the user explicitly
  confirms an owner/admin exception and the reason is reported.

## Version Policy

- Require an appropriate firmware version bump for dependency updates,
  PlatformIO configuration changes, firmware code changes, or other changes
  that affect build outputs unless the user explicitly says not to bump.
- Governance-only and documentation-only changes do not require a version bump.
  Report that the version bump was skipped by policy.
- Use patch version bumps for dependency updates, bug fixes, internal compatible
  changes, and build or configuration maintenance with no public behavior break.
- Use minor version bumps for new compatible firmware features or behavior.
- Use major version bumps for incompatible settings/NVS layout changes or
  behavior changes requiring user migration.
- `APP_VERSION` and `VERSION_DATE` in `src/settings.h` are the firmware version
  declarations for BoilerSaver.
- Dependency versions in `platformio.ini` are independent dependency metadata,
  not the firmware version source of truth.
- Before changing versions, dependency metadata, or files that may contain the
  firmware version, scan the repository and report candidate files found.
- Version scans must include at least:
  - `src/settings.h`
  - `platformio.ini`
  - `readme.md`
  - relevant files under `docs/`
  - any additional files found by ripgrep containing the current or target
    version string
- Suggested version scan commands:
  - `rg -n "version|VERSION|APP_VERSION|VERSION_DATE" .`
  - `rg -n "<current-version>|<target-version>" .`
- Keep duplicate dependency declarations in the `usb` and `ota` environments
  synchronized when intentionally updating a dependency.
- If version declarations disagree before work starts, report the mismatch
  before changing version-related files.
- If the intended target version is ambiguous, stop and ask for clarification
  instead of guessing.
- After any version-related change, report:
  - the firmware version found in `src/settings.h`
  - synchronized dependency declarations when applicable
  - files intentionally not changed and why
  - commands used for version scanning
  - build/test validation performed
  - any remaining mismatch or risk

## Session-Close Workflow

- Run session-close actions only when the user explicitly requests the named
  workflow or clearly requests commit and publication.
- Default explicit session-close behavior:
  - commit the current work branch with `git add -A` and a meaningful English
    commit message
  - push the work branch to `origin`
  - run `pio run -e usb` when `.cpp`, `.h`, `platformio.ini`, build flags,
    dependencies, or generated firmware assets changed
  - skip the firmware build for governance-only or documentation-only changes
    and report why

## Code Rules

- Code comments must be in English.
- Identifiers and function names must be in English.
- Error and log messages must be in English.
- Emojis are forbidden in code, comments, logs, and generated outputs.
- Favor small, coherent changes over broad speculative refactors.
- Keep hardware-facing and configuration-sensitive changes conservative.

## Documentation Exception

- Markdown prose is exempt from flash-oriented logging brevity rules.
- Markdown prose may use readable long-form tags such as `[WARNING]`, `[NOTE]`,
  and `[INFO]`.
- Code blocks inside Markdown are not exempt. Code examples and text log
  examples must still follow the global logging policy from this file.
- Documentation must still be written in English unless the user explicitly
  requests a different language for a specific document.

## Logging Policy

All source code, headers, tests, and code blocks inside Markdown must follow this
policy.

- Log messages must use short ASCII-only severity tags when severity tags are
  embedded in text:
  - `[E]` error
  - `[W]` warning
  - `[I]` info
  - `[D]` debug
  - `[T]` trace
- Do not embed redundant long tags such as `[ERROR]`, `[WARNING]`, `[INFO]`,
  `[DEBUG]`, or `[SUCCESS]` when the logging API already carries severity.
- Verbose is not a severity level. Verbose controls whether logs are emitted;
  severity describes impact.
- Severity reassignment is allowed when technically justified.
- Log message updates are not public API renames.
- Keep log text semantically clear and as short as reasonably possible to reduce
  flash usage.
- Prefer common abbreviations such as `cfg`, `init`, `ok`, `fail`, `adc`, `pwm`,
  and `io` where clarity remains intact.
- Do not repeat information already encoded by severity, module prefixes, or
  surrounding context.
- IO and hardware-level logs are usually debug or trace. Use warning for notable
  recoverable conditions and error for critical failures.

## Rename And Logging Interaction

- API renames and logging normalization are separate concerns.
- Do not mix logging normalization into API rename phases.
- Before any API rename, perform a full reference search with rg.
- After renaming, rerun rg to ensure old names do not remain in relevant
  existing project paths such as `src/`, `test/`, and `docs/`.

## Tool Policy

- Prefer available VS Code or agent-workspace tools first when they fit the task.
- Prefer locally installed CLI tools next.
- Do not use unnecessarily heavy tools when simple search or inspection is
  enough.
- Known local tools on the user's Windows/PowerShell environment may include:
  - git
  - gh
  - rg
  - fd
  - jq
  - dasel
  - jc
  - 7z
  - pwsh
  - winget
  - choco
  - coreutils
  - node / npm
  - python
  - uv
  - platformio / pio, if installed for this repository
- Preferred tools:
  - rg for text search, audits, and reference checks.
  - fd for file discovery, audits, and reference checks.
  - git for version-control inspection and explicit version-control actions.
  - platformio / pio for PlatformIO build, upload, monitor, and test flows.
  - gh for GitHub PRs, CI checks, issues, and GitHub operations when available.
  - jq for JSON.
  - dasel for YAML, TOML, JSON, or XML inspection when it is the safest fit.
- When searching for governance, agent instructions, workflow shortcuts, branch
  rules, pull request rules, issue rules, validation rules, version rules, or
  repository policy, include hidden paths.
- For governance searches with rg, use `rg --hidden`.
- For governance discovery with fd, use `fd --hidden`.
- Plain `rg ... .` or plain `fd ... .` is not sufficient for governance
  discovery because hidden directories such as `.github/` may be skipped.
- When governance files are known by path, prefer reading those files directly
  over discovering them through repository-wide search.
- If rg or fd is missing, report that and give a simple install hint instead of
  silently using a weaker search path for audits or reference checks.
- When reporting search results, include the rg pattern used and state whether
  hidden paths were included when governance, policy, workflow, or agent files
  were in scope.
- Do not assume every listed tool is installed on every machine. If a required
  tool is missing, report the failed command clearly.
- Do not install or upgrade tools unless the user explicitly asks.
- On Windows, do not assume sed or awk are available. Prefer PowerShell-native
  commands unless availability was confirmed.
- Shell command quality:
  - Prefer simple robust commands over complex shell-escaped one-liners.
  - On Windows/PowerShell, prefer single quotes for regex/search patterns.
  - Avoid PowerShell backticks in search patterns unless required.
  - If quoting fails, retry once with safer quoting; do not repeat equivalent
    broken commands.
  - If quoting remains unclear, simplify, split the search, or read known files
    directly.
  - Do not treat failed commands as evidence about repository content.
  - Distinguish command failure, no matches, missing files, and missing tools in
    reports.
  - Example: `rg --hidden -n 'workflow\.begin|workflow\.audit' .github`.
- Use dasel for YAML, TOML, JSON, or XML inspection and edits when it is the
  safest fit.
- Use jq for JSON.
- Use gh for GitHub operations when authenticated and available.
- Use 7z for archive inspection or extraction when needed.
- Use uv only when Python tooling is actually part of the task. Do not infer
  Python project workflows from the presence of uv.
- Do not mix jq and dasel syntax.
- Do not use deprecated dasel flags.
- Prefer structured tools over brittle text parsing when that reduces risk.
- Do not assume repository, branch, PR, or governance state. Verify repository
  state with git and gh, and verify governance state by explicitly reading the
  relevant governance files.

## Validation Baseline

- Use configured and enabled GitHub Actions or checks when they exist.
- Do not invent required CI workflows.
- If no enabled CI is configured, report that and rely on required local
  validation.
- Run at least one PlatformIO build after `.cpp`, `.h`, `platformio.ini`, build
  flag, dependency, or generated firmware asset changes.
- Default build check:
  - `pio run -e usb`
- If only Markdown, governance, or ignore files changed, skip PlatformIO build
  unless the user explicitly asks for it and accepts the cost.
- Governance-only changes require a governance consistency check, but do not
  require PlatformIO validation by themselves.
- A governance consistency check means reading the affected governance files and
  verifying that agent routing, shortcut rules, branch rules, tool policy,
  validation rules, version rules, and reporting rules do not contradict each
  other.
- Governance-only validation should include:
  - `git diff --check`
  - `git status --short`
  - stale-reference scans with `rg --hidden`
  - representative `git check-ignore -v` checks when ignore rules changed
- If tests are affected, run `pio test` for at least one relevant environment.
- Run relevant tests when tests are present and affected.
- Docker or image builds are not required unless configured in this repository
  or explicitly in scope.
- Upload and serial monitor commands require explicit user request because they
  interact with hardware or network devices.
- If a required validation cannot run, report that plainly.

## Mandatory Reporting

After file-changing work, report:

- branch used
- governance files read when governance, workflow, branch, PR, merge,
  validation, version, or cleanup rules were in scope
- files changed
- concise summary of what changed
- assumptions removed or rewritten and the reason
- validation run
- skipped validation with reason
- remaining risks or blockers

Inspect relevant diffs before reporting file-changing work. Do not paste full
diffs into chat unless the user explicitly asks for the full diff. Prefer
`git diff --stat`, changed file lists, and focused summaries. Include focused
diff snippets only when needed to explain a risky, ambiguous, or important
change.

## Final Rule

- Never mark an issue as solved or fixed until the user explicitly confirms it
  works.
