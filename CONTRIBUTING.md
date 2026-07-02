# Contributing to GlanceHUD

Thanks for your interest! This guide covers the workflow and quality bar.

## Golden rule: keep `core/` pure

`core/` must **never** depend on Android, iOS, or any platform SDK. Platform code
lives in `android/`, `ios/`, `flutter/`, `react-native/` and talks to the core
through the interfaces (`IMetricProvider`, `IOverlayRenderer`) and the C ABI (`Api.h`).

## Development setup

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset ci
```

## Code style

- **C++20**, RAII, smart pointers, no raw `new`/`delete`, no unnecessary allocation
  on the hot path.
- Formatting is enforced by `clang-format` (`.clang-format`). Run before committing:
  ```bash
  find core -name '*.hpp' -o -name '*.cpp' | xargs clang-format -i
  ```
- Static analysis via `clang-tidy` (`.clang-tidy`). Fix warnings you introduce.
- Public headers must carry **Doxygen** comments.

## Commit & PR

- Branch off `main`; use focused, reviewable PRs.
- Write descriptive commit messages (imperative mood).
- Add or update tests for behavior changes; run them locally (`ctest --preset ci`).
- Update `CHANGELOG.md` under `## [Unreleased]`.

## Tests

Unit tests live in `core/tests/` (GoogleTest). Add a test for every new engine
behavior. Prefer injecting `IClock` for deterministic timing tests.

## Reporting bugs / requesting features

Open an issue with a minimal reproduction, platform, and expected vs. actual behavior.
