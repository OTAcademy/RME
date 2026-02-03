# Palette's Journal

## 2024-10-26 - Property Flags Lack Context
Learning: Many property checkboxes (e.g., Unpassable, Unmovable) rely on domain knowledge. Adding tooltips clarifies their impact (e.g., "Item blocks movement").
Action: Always audit property inspectors for missing tooltips on boolean flags.

## 2024-05-22 - Modal Dialog Overuse
Learning: The application frequently uses modal dialogs for simple success feedback (e.g., "Item added to tileset", "Items removed"), which disrupts workflow.
Action: Replace success-only modal dialogs with non-intrusive status bar updates (`g_gui.SetStatusText`) where possible.

## 2024-05-23 - Dialog Buttons Lack Feedback
Learning: Standard "OK/Cancel" buttons in dialogs often lack context. Users may hesitate if they are unsure if "OK" saves or just closes.
Action: Add tooltips to "OK" and "Cancel" buttons in all custom dialogs to clarify the action (e.g., "Apply changes and close", "Discard changes").
