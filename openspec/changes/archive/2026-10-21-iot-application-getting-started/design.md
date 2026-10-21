## Context

The `IoTApplication` library has a README covering the API surface but no entry-point guide for developers starting a new project. See proposal.md for motivation.

The library lives at `G:\IoT\libraries\IoTApplication`. The existing README is `README.md` at the repo root. No `docs/` directory exists yet.

## Goals / Non-Goals

**Goals:**
- Single markdown file `docs/getting-started.md` in the `IoTApplication` library repo
- Cover the full path from empty project to a fully-featured application, in logical sections that build on each other
- Each section pairs a short recipe (minimal code / config) with a reference table so the document is useful both as a tutorial and as a lookup
- Cover all `platformio.ini` build flags and `lib_deps` entries a developer needs to know about
- Section 7 (Remote OTA) is detailed: manifest format, post-build script, upload pipeline, server layout, multi-language builds
- Link from `README.md` in the same repo

**Non-Goals:**
- Not a full API reference — the README already covers that
- Not a tutorial for PlatformIO or Arduino basics — target audience already knows these
- Not documentation for `ESPAsyncWiFiManager` internals or `ArduinoHA` — link to those projects where appropriate
- `WM_SUPPORT_TRAIN_CONTROL` is application-specific and excluded

## Decisions

### D1: Single file vs. multi-file docs

**Decision:** One file (`docs/getting-started.md`).

**Rationale:** The guide is progressive — each section depends on the previous. A single file keeps navigation simple, works well on GitHub, and avoids broken cross-links. If the guide grows significantly it can be split later.

**Alternative considered:** Multiple files under `docs/` (one per section). Rejected because it adds navigation overhead without benefit at this size.

### D2: Document structure — spine of tutorial with anchored reference tables

**Decision:** Nine named sections in progressive order (minimal → HA → RTC → display → sensors → OTA → geo → localisation) followed by a full flags reference appendix. Each section contains a minimal code/config snippet plus a reference table for that section's flags and deps.

**Rationale:** A developer starting fresh reads top to bottom. A returning developer jumps directly to the relevant anchor. The appendix gives a single place for a full flags lookup without duplicating prose.

### D3: OTA section depth

**Decision:** Section 7 covers the full pipeline in detail: build flags, `post_manifest.py` output, `manifest.json` format and accumulation behaviour, server directory layout, upload scripts, and multi-language builds.

**Rationale:** Remote OTA is the most complex setup step and is not documented anywhere other than the source code. All other major topics (HA integration, NTP, sensors) are documented in their own library READMEs and can be referenced.

### D4: Sensor sections depth

**Decision:** DS18B20 and BME280 sections show the minimal `lib_deps` and constructor, then link to each library's own README for full detail.

**Rationale:** `IoTDS18B20` has a comprehensive README covering its web config UI, REST API, NVS persistence, and iCache workaround. Duplicating that here would create maintenance burden.

## Risks / Trade-offs

- [Staleness] Build flags or lib_deps versions documented here can drift from the actual library. Mitigation: keep the guide close to the library source; note in the document that versions shown are examples and the latest should be checked on the respective library page.
- [Scope creep] The OTA section is intentionally detailed; if the upload pipeline changes (e.g., new upload script added), the guide needs updating. Mitigation: the guide describes the mechanism (manifest format, server layout) rather than the specific scripts, so minor script changes don't invalidate the doc.
