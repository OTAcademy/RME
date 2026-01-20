---
description: Rules to prevent lazy implementations, placeholders, and hallucinations.
---

# Agent Behavior & Integrity Rules

## 1. NO Placeholders or "TODO" Implementations
- **NEVER** write `// Implement logic here` or `// ... rest of function`
- Write the **FULL** implementation
- If complex, break it down and write every line
- **Exception**: Only if user explicitly asks for "skeleton" or "mock"

## 2. Exhaustive Implementation
- If task requires editing 5 files, edit all 5
- Don't edit 1 and say "do the rest similarly"
- If running out of context, **STOP** and tell user to continue in next turn
- Never hallucinate that you finished

## 3. Verify, Don't Assume (Anti-Hallucination)
- Don't claim "fixed" until verified
- Say: "I have applied a fix, now verifying"
- If grep finds nothing, **ADMIT IT**
- If can't read a file, say so - don't guess contents

## 4. No Lazy Refactoring
- When changing a signature, update **ALL** callers
- Search for dependencies **BEFORE** breaking changes
- Run grep to find all usages

## 5. Respect Project Rules
Before writing code, verify against:
- `.agent/rules/wxwidgets.md` - UI patterns
- `.agent/rules/cpp_style.md` - C++ standards

If user request violates rules:
- **REFUSE** and propose compliant solution
- Example: "I will not add this to Application.cpp - I'll create a proper controller"

## 6. Incremental Modernization
When touching existing code:
- Apply C++20 improvements opportunistically
- Replace `NULL` → `nullptr`
- Add missing `override`
- Use range-based for loops
- Don't break existing functionality while modernizing
