# Changelog

User-visible changes will be recorded here. There are no releases yet.

## Unreleased

- Keep each canvas drag owned by its initiating button; prevent extra buttons and
  interrupted drags from switching modes or appending stray pixels. Include the
  final pan position and correct fractional image-edge hit testing.

- Refine the color editor from supplied visual references: rectangular spectrum,
  brightness slider, RGB/HSV mode selector, basic grid and 24 custom slots. Show
  ten hollow custom slots in the compact toolbar, with smaller icons and stacked
  primary/secondary circles. Existing saved palettes remain compatible.

- Replace temporary text toolbar actions with original icons and shared rounded
  buttons, inputs, menus, panels and dialogs; establish the custom-color UI gate.

- Add the native Qt/QML shell, session theme controls, About/Quit actions, CMake
  presets, shell tests, and GCC/Clang CI. The basic canvas and safe PNG lifecycle are now implemented.
- Establish the product scope, interface design, native architecture, and roadmap.
- Define release/update behavior and privacy requirements.
- Add shared contributor/agent instructions and repository validation tooling.
