# CM 4.10.5 and Lightweight Live Implementation Plan

**Final status:** The 4.1.0 candidate failed the production gate with ESP_RST_PANIC. Production was recovered to 4.0.4 with all 47 original settings retained. See [failure and coverage analysis](2026-10-31-production-gate-failure.md). The passing bench evidence below is not production approval.

> **For agentic workers:** Use superpowers:executing-plans. Parent review precedes production rollout.

**Goal:** Upgrade BoilerSaver 4.0.4 to released CM 4.10.5 with demand-driven Live enabled and existing production heating, GPIO bindings, NVS and MQTT identities preserved.

**Architecture:** Existing named providers and Live builders already support exact group selection. Pin the released package and prevent implicit local source replacement. A compile-time bench environment isolates COM8 hardware and network identity; production retains existing hardware/control paths.

**Tech Stack:** ESP32 Arduino, PlatformIO, CM 4.10.5, existing AsyncTCP, sensor and display libraries.

**Spec:** User-authorized BoilerSaver upgrade; COM8 first, at least 300 seconds without panic or unexpected reset, parent review before 192.168.2.130.

## Constraints and Review Focus

- No production writes, commits, pushes, merges or releases in this implementation task.
- Bench: DHCP, distinct app/client/topic namespace, no physical GPIO/display/sensor access.
- Bench sensor/control data must identify its [MOCKED!] provenance; it cannot prove physical heating.
- Preserve production settings and existing recovery firmware; never reset production NVS.
- Empty runtime selection must omit ordinary providers; only active page groups may execute.
- Builds must not silently substitute neighboring CM sources.

## Tasks

- [x] Record read-only production identity and configuration.
- [x] Test then replace the build wrapper with package-only precompile; pin CM 4.10.5.
- [x] Bump BoilerSaver to user-requested 4.1.0 and add isolated COM8 bench support.
- [x] Build USB, OTA and bench; verify package versions and firmware hashes.
- [x] Upload only bench to COM8; check identity, isolation, selective/empty Live, browser UI.
- [x] Run at least 300 seconds of serial/runtime observation, review diff and hand evidence to parent.

## Validation Result

- USB, OTA and isolated bench builds passed with CM 4.10.5 and Live enabled.
- Two package-wrapper tests passed after a demonstrated red state.
- Seventeen production hardware/control/MQTT functions and all settings structures match the baseline.
- COM8 completed a 300.032-second gate with one intentional boot, no fault/reset events and no observer errors.
- HTTP empty/unknown selection: 66 bytes; Boiler selection: 351 bytes; full reference: 755 bytes. Selected WebSocket and page-scoped metadata/layout passed.
- Chrome Boiler/System/Settings navigation and simulated Shower ON/OFF passed; final simulated state is OFF with timer zero.
- All 47 existing production configuration fields remained unchanged during the bench exercise.
- Evidence and recovery artifacts remain ignored under .pio/evidence/. Production was not deployed by this implementation agent. Physical sensor/display/heating acceptance remains outside the mocked bench proof.
