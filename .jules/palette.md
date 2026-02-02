# Palette's Journal

## 2024-10-26 - Property Flags Lack Context
Learning: Many property checkboxes (e.g., Unpassable, Unmovable) rely on domain knowledge. Adding tooltips clarifies their impact (e.g., "Item blocks movement").
Action: Always audit property inspectors for missing tooltips on boolean flags.

## 2024-05-22 - Modal Dialog Overuse
Learning: The application frequently uses modal dialogs for simple success feedback (e.g., "Item added to tileset", "Items removed"), which disrupts workflow.
Action: Replace success-only modal dialogs with non-intrusive status bar updates (`g_gui.SetStatusText`) where possible.
