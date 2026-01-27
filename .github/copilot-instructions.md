# AI Agent Instructions for Federated Project

When updating project documentation and code:

## 1. Verify Claims Against Actual Code

ALWAYS verify all claims about implementation status and metrics:

```bash
# Run tests to get exact counts
cd build && ./federated_tests | grep "Summary:"

# Count test files
find tests/ -name "*.cpp" -exec grep "^TEST(" {} \; | wc -l

# Check implementation files
ls -la src/ include/ tests/

# Review documentation
ls -la docs/
```

## 2. Mark Implementation Status Clearly

Use consistent markers throughout documentation:

- **(FULL)** - Fully implemented and tested, production-ready
- **(STUB)** - Documented stub with TODO markers, include reason (e.g., "requires platform APIs")
- **No marker** - Planned for future implementation

## 3. Calculate Percentages Accurately

Always provide both completion metrics:

- **With stubs**: Count of items with any code (including stubs)
- **Fully functional**: Count of items that are production-ready

Example: "82% - 9 of 11 with stubs, 45% fully functional - 5 of 11"

## 4. Keep Test Metrics Current

Verify and update test counts:

- **Total tests** = unit tests + scenario tests
- Verify assertion counts from actual test runs
- Update breakdown: X unit + Y scenario = Z total

## 5. Document New Features Immediately

When adding new implementations:

- Add to appropriate phase/section in README
- Include test counts and RFC references where applicable
- Update completion percentages
- Update "Last Updated" date

## 6. Maintain Consistency

Keep documentation organized:

- Use the same format for similar items
- Keep phase numbering sequential
- Remove duplicate or outdated sections
- Ensure cross-references are accurate

## 7. Cross-Reference Documentation

Maintain consistency across files:

- Update STATUS.md, IMPLEMENTATION_SUMMARY.md when needed
- Ensure README.md reflects current implementation
- Keep docs/ files synchronized with code changes

---

## Project Standards

### Test-First Development
- All features start with failing tests
- Maintain 100% test coverage
- Include scenario tests for user-facing features

### Zero Dependencies
- Only libc and OS syscalls
- No external runtime dependencies
- Pure C++17 implementation

### RFC Compliance
- Document RFC references in header comments
- Follow specifications precisely
- Note any deviations explicitly

### Flat Architecture
- No deep inheritance hierarchies
- Explicit error handling (no exceptions)
- Clear ownership and state management

---

**This file guides all AI agents working on the Federated project. Follow these guidelines to maintain code quality and documentation accuracy.**
