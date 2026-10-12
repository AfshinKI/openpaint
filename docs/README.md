# Documentation map

These documents are implementation contracts. Plans describe intended behavior;
[FEATURES.md](FEATURES.md) records what has actually been built and verified.

| Need | Start here |
| --- | --- |
| Build/run the current native shell | [Build guide](BUILD.md) |
| Product goals and release order | [Roadmap](ROADMAP.md) |
| Modern Paint scope and reference evidence | [Feature checklist](FEATURES.md) |
| Layout, visual rules, shortcuts, interactions | [Interface design](DESIGN.md) |
| Language, modules, rendering, document and save model | [Architecture](ARCHITECTURE.md) |
| How to implement a feature | [Development guide](DEVELOPMENT.md) |
| First executable implementation plan | [M1 foundation](plans/0001-foundation.md) |
| Testing and release gates | [Quality](QUALITY.md) |
| Download formats and Updates screen | [Releases](RELEASES.md) |
| Personal data, network access, fixture rules | [Privacy](PRIVACY.md) |
| Why this stack | [Decision 0001](decisions/0001-native-stack.md) |
| New plans and decisions | [Feature template](templates/FEATURE.md), [decision template](templates/DECISION.md) |
| External source material | [References](REFERENCES.md) |

Update the responsible document in the same PR as its behavior. A roadmap entry
does not override a detailed contract silently: document the change and its reason.
Maintain one canonical explanation and link to it rather than copying rules.
