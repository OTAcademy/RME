
# MISSION: Refactor `\source\ui\old_properties_window.cpp` for Separation of Concerns, Modernization, and Quality

You are a **code refactoring specialist**. Your job is to find **THREE** low-hanging fruit separation of concerns violations in **`\source\ui\old_properties_window.cpp`** and **FIX** them.

Additionally, you must:
1. **Identify and reduce** code smells in the code you touch.
2. **Identify and fix** KISS (Keep It Simple Stupid) and DRY (Don't Repeat Yourself) violations.
3. **Modernize** the code to use **C++20 standard or newer** (make it quicker, better, shorter).

**LONG-RUN OBJECTIVE:** Introduce separation of concerns in `\source\ui\old_properties_window.cpp`, keeping files under 500 lines of code, while significantly improving code quality and modernizing syntax.

---

## WORKFLOW

### PHASE 1: UNDERSTAND THE CODEBASE (15 minutes)

Scan the entire codebase to understand:

* What services/classes already exist in this domain?
* What is each component responsible for?
* What patterns are used for organizing code?
* Where do similar concerns live currently?

**Pay special attention to:**

* Related classes and modules
* Utility classes and helpers
* Core system management classes
* Data structures and models
* How concerns are currently separated

### PHASE 2: ANALYZE `\source\ui\old_properties_window.cpp` (10 minutes)

Read **`\source\ui\old_properties_window.cpp`** completely. Identify **ALL** responsibilities currently in `\source\ui\old_properties_window.cpp`:

* What does `\source\ui\old_properties_window.cpp` do?
* What concerns are mixed together?
* What doesn't belong in this component?
* **Where are the Code Smells?**
* **Where are KISS/DRY violations?**
* **Where can C++20 features be applied?**

List every distinct concern you find. Be specific about what code handles what responsibility.

### PHASE 3: PICK LOW-HANGING FRUIT (5 minutes)

Choose **THREE** concerns to extract based on:

* It clearly doesn't belong in `\source\ui\old_properties_window.cpp`
* There's an existing class that handles similar concerns **OR** it's obviously a new concern that should be separate
* It can be moved with minimal risk (<500 lines of code)
* It will make `\source\ui\old_properties_window.cpp` cleaner and more focused
* **It offers opportunities to apply C++20 modernization and fix code smells during the move.**

**Examples of what to look for:**

* Data validation logic that should be in a validator class
* File I/O operations that should be in a data access layer
* Business logic that should be in a service class
* UI logic mixed with business logic
* Configuration management that should be separate
* Caching logic that should be in a cache manager
* Logging/diagnostics that should be centralized
* State management that should be in a state manager
* Parsing/formatting that should be in utility classes
* Third-party API interactions that should be wrapped

> **Do NOT pick:** Core functionality that `\source\ui\old_properties_window.cpp` legitimately owns, coordination logic that is the component's responsibility, or the main workflow that defines the component's purpose.

### PHASE 4: DECIDE WHERE IT GOES (5 minutes)

For each concern, determine:

* Does an existing class handle this concern? → **Move it there**
* Is this a new concern? → **Create new class/file**
* What should the class be called?
* What methods should be exposed?
* What dependencies will it have?

### PHASE 5: REFACTOR THE CODE (30 minutes)

Execute the refactoring:

#### If moving to existing class:

1. Add new method(s) to the existing class
2. Move the code from `\source\ui\old_properties_window.cpp` to the class method
3. **Refactor the moved code:**
    *   **Reduce** code smells.
    *   **Fix** KISS and DRY violations.
    *   **Modernize** to C++20 (e.g., `std::span`, `std::format`, ranges, concepts, `auto`).
4. Update `\source\ui\old_properties_window.cpp` to call the class method
5. Update includes/imports
6. Update API documentation

#### If creating new class:

1. Create appropriate new files (`.h`/`.cpp`, `.ts`, `.java`, etc.)
2. Move the concern code into the new class
3. **Refactor the moved code:**
    *   **Reduce** code smells.
    *   **Fix** KISS and DRY violations.
    *   **Modernize** to C++20 (e.g., `std::span`, `std::format`, ranges, concepts, `auto`).
4. Create clean interface methods
5. Update `\source\ui\old_properties_window.cpp` to use the new class
6. Add to build configuration if needed
7. Update includes/imports

#### Always:

* Keep the behavior **EXACTLY** the same (refactor, don't rewrite logic unless fixing smells)
* **Modernize** the code to C++20 standards
* **Fix** KISS and DRY violations in the moved code
* **Reduce** code smells
* Preserve all existing functionality
* Update any affected includes/imports
* Maintain existing error handling

### PHASE 6: VERIFY (10 minutes)

Build and test the project:

```bash
use build_windows.bat
```

* Ensure no compilation/build errors
* Verify functionality still works correctly
* Check that behavior is unchanged

---

## RULES

1. **Pick THREE concerns only** - don't try to fix everything
2. **Refactor and Improve** - Move code, but also clean it up AND modernize it.
3. **Use existing patterns** - follow the codebase conventions unless they are outdated (then modernize).
4. **Modernize to C++20** - Use modern C++ features where appropriate (quicker, better, shorter).
5. **Fix Smells, DRY & KISS** - actively reduce code smells and complexity.
6. **Don't break functionality** - verify behavior is preserved.
7. **Document as you go** - update comments and documentation.

---

## DELIVERABLES

1. **Planning Artifact**: Analysis of the codebase and selected concerns, including identified smells and modernization opportunities.
2. **Task Artifact**: Step-by-step refactoring checklist.
3. **Refactored Code**: The actual code changes (Modernized C++20, separated concerns).

---

**START NOW.** Begin by scanning the codebase to understand existing architecture. Then read **`\source\ui\old_properties_window.cpp`**, pick three concerns to extract, and execute the refactoring. **Always create a planning artifact and task artifact.**

Remember that is better to put new files into new folders for better organization.

this time focus on 3 BIG fruits, like very heavy structure that dont belong here