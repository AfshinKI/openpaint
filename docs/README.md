# Documentation map

These documents are implementation contracts. Plans describe intended behavior;
[FEATURES.md](FEATURES.md) records what has actually been built and verified.

| Need | Start here |
| --- | --- |
| Build/run the current editor | [Build guide](BUILD.md) |
| Product goals and release order | [Roadmap](ROADMAP.md) |
| Modern Paint scope and reference evidence | [Feature checklist](FEATURES.md) |
| Layout, visual rules, shortcuts, interactions | [Interface design](DESIGN.md) |
| Language, modules, rendering, document and save model | [Architecture](ARCHITECTURE.md) |
| How to implement a feature | [Development guide](DEVELOPMENT.md) |
| First executable implementation plan | [M1 foundation](plans/0001-foundation.md) |
| M1 completion evidence and performance baseline | [M1 closure](plans/0009-m1-closure.md) |
| Testing and release gates | [Quality](QUALITY.md) |
| Linux/macOS installation and removal | [Install guide](INSTALL.md) |
| Current release and platform evidence | [v2 release plan](plans/0062-macos-v2-release.md) |
| Intermediate changes and superseded decisions | [Release progression](ROADMAP.md#release-progression) |
| Historical alpha ribbon/color polish | [Eight-step UI plan](plans/0028-alpha-ui-polish.md) |
| M2 completion and release validation | [Alpha release plan](plans/0027-alpha-release.md) |
| Download formats and Updates screen | [Releases](RELEASES.md) |
| Personal data, network access, fixture rules | [Privacy](PRIVACY.md) |
| Why this stack | [Decision 0001](decisions/0001-native-stack.md) |
| Raster layers and OpenRaster | [Layer feature plan](plans/0058-layers.md), [bounded model decision](decisions/0008-bounded-layer-projects.md) |
| New plans and decisions | [Feature template](templates/FEATURE.md), [decision template](templates/DECISION.md) |
| External source material | [References](REFERENCES.md) |

Update the responsible document in the same PR as its behavior. A roadmap entry
does not override a detailed contract silently: document the change and its reason.
Maintain one canonical explanation and link to it rather than copying rules.

Plans and decisions are historical evidence as well as contracts. Old checkpoint
phrases such as “PR deferred” or “next: M1” do not override the current feature
ledger or release progression. Preserve dated evidence and outstanding manual
limits when reconciling a plan; do not relabel old captures or tests as v2 results.
