## Why

The IoTApplication library has no entry-point documentation for developers who want to build a new application on top of the framework. Without a guide, a new developer must reverse-engineer the demo project to discover which libraries are mandatory, which are optional, and how the build flags interact — work that should be done once and shared.

## What Changes

- New file `docs/getting-started.md` added to the `IoTApplication` library repository
- `README.md` in `IoTApplication` gains a link to the new guide
- No code or API changes

## Capabilities

### New Capabilities

_None — this is a pure documentation change. `skip_specs: true` is set in `.openspec.yaml`._

### Modified Capabilities

_None._

## Impact

- `G:\IoT\libraries\IoTApplication\docs\getting-started.md` — new file
- `G:\IoT\libraries\IoTApplication\README.md` — add one link line
- No library consumers are affected; purely additive documentation
