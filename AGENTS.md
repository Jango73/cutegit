# AGENTS.md

This file provides guidance to agents when working with code in this repository.

## Project Overview
`CuteGit` is a graphical versioning client (C++/Qt/QML), built with qmake
for Qt6 (see `README.md`). Layout:
- `qt-plus/` (git submodule, branch `qt6`): shared Qt utility library. Check
  here first before adding helpers. Consumed as a compiled library
  (`libqt-plus[d].so`, built in-source into `qt-plus/bin/` by
  `qt-plus/build.sh`) plus headers (`INCLUDEPATH`), and via `functions.pri`.
  `qt-plus-nolib-web.pri` exists for consumers that need a source subset.
- `CuteGit/` (application): `CuteGit.pro` (subdirs) + `CuteGit.pri`, sources in
  `CuteGit/sources/` (`CController`, `CRepository`, models, `commands/`),
  QML UI in `CuteGit/qml/`, `deploy/`.
- `scripts/linux/`: `build.sh`, `clean.sh`, `deps.sh`, `run.sh`, `smoke.sh`,
  `package.sh`, `set-version.sh`. `scripts/windows/`: `build.bat`, `run.bat`.
- `Media/`: assets.

Fresh clones have an empty `qt-plus/` until submodules are initialized
(`git submodule update --init --recursive`). The application targets **Qt6**
(Qt 6.2.4 on this machine); the reference Linux build is qmake6 + make via
`scripts/linux/`.

## Main rule
**If the guidelines below are not followed, all modifications will be rejected.**

## Communication Guidelines
- NEVER use emojis in responses.
- DON'T act like a human being with emotions, just be a machine.
- DON'T says "Great!", "Perfect!", "You're right" all the time.
- If a demand DOES NOT make sense (for instance, breaks an architecture instead of refactoring it), SAY IT and ask for confirmation BEFORE DOING ANYTHING.
- NEVER create a commit unless the user explicitly asks for it in the current conversation.

## Architecture and Reuse Rules
- Bidirectional coupling is **STRICTLY FORBIDDEN**, both when writing code from scratch and when delivering a fix. Keep dependencies unidirectional and break cycles instead of introducing or preserving them. This applies to headers first: no inclusion cycles, even "managed" by forward declarations (Qt6 `moc` needs complete types, see below).
- Ownership flows `CController` -> `CRepository` -> models. Back-pointers use plain members, never `Q_PROPERTY` (which would reintroduce a header cycle).
- Any behavior likely to appear in multiple places MUST be implemented as a reusable module in `qt-plus/source/cpp`, not duplicated in the application.
- **Container usage**: use Qt containers (`QVector`, `QMap`, `QStringList`) where they exist. Do not introduce custom linked-list infrastructure.
- **Qt6 moc completeness**: every type used in a class's own `Q_PROPERTY`, signals and slots MUST be completely defined in that header (directly or transitively). Forward-declared pointer types are NOT enough: `moc` generates static metatype code requiring `sizeof`. When adding a property over an application type, include its header; if that creates a cycle, remove the property or break the cycle first.
- **No globals**: Before adding a global variable, **ALWAYS ASK** if permitted.

## Coding Conventions
- **Types**: Use Qt types (`QString`, `QMap`, ...) where they exist instead of introducing parallel raw-C representations.
- **Dependencies**: This is a Qt user-space project. Qt and the STL are allowed. Do NOT add new third-party libraries/modules (vendored or system) without asking.
- **TEXT literals**: Write log/UI messages directly as `QString`.
- **Declaration order**: Group declarations by type. 1: macros / 2: type definitions / 3: inline functions / 4: external functions / 5: other
- **Function order**: DO NOT OVERUSE forward declarations. Define functions before they are used.
- **I18n**: Write comments, console output and technical doc in english.
- **Naming**: PascalCase for classes/variables/members (`C`-prefixed classes, `m_`/`g_`/`p_` prefixes as in the existing code), SCREAMING_SNAKE_CASE for structs/defines.
- **Naming clarity**: In addition to using full words, every name must express its intent clearly and without ambiguity.
- **Comments**: For single-line comments, use `//`, not `/*`.
- **Style**: 4-space indentation. Match surrounding code instead of reformatting whole files.
- **Numbers**: Hexadecimal for constant numbers, except for sizes, vectors, points and time.
- **Number suffixes**: Do not add numeric suffixes like `u` to constants; they are not wanted here.
- **Documentation wording**: Use timeless technical wording. Do not use temporal terms like "now", "currently", "at this time" in documentation/comments.
- **Languages**: C++ for the application, QML for the UI, avoid Python.
- **Unused parameters**: Use `Q_UNUSED()` to suppress the "unused parameter" warning.
- **Clean code**: No duplicate code. Create intermediate functions to avoid it. This also applies to data: create intermediate structures to avoid duplicating data.
- **Functions**: Add a doxygen header to functions and separate all functions with a 75 character line such as : /************************************************************************/
- **File size**: Keep source files under 1000 lines; split by responsibility before crossing this limit.
- **Vocabulary**:
  - NEVER use abbreviations; ALWAYS use full words (acronyms are OK).
  - No figurative language: respect the repository vocabulary, use every word
    in its primary (literal) sense. Name things for what they are, not for
    what they resemble.
  - No "directory" : use "folder".

### Commit message
The agent always signs its commits: append a last line `Agent: <agent-name>`
(for example `Agent: opencode/muse-spark`) so authorship stays identifiable
in the log, whatever the commit prefix above. Never leave a blank line in
the commit message: lines follow each other directly.

## Common Build Commands
- Dependencies: `./scripts/linux/deps.sh` (submodules + apt/dnf/pacman packages;
  needs `sudo`, which prompts for a password here)
- Build (shadow): `./scripts/linux/build.sh [--debug|--release]`
  (`qmake6 CuteGit.pro CONFIG+=<debug|release>` in `build-<debug|release>`,
  then `make -j`). Builds `qt-plus` (`libqt-plus[d].so` in `qt-plus/bin/`),
  then `CuteGit` (`CuteGitApp[d]` in `build-<debug|release>/CuteGit/bin/`).
- Clean: `./scripts/linux/clean.sh [--debug|--release|--all]`
- Run (interactive GUI, blocks): `./scripts/linux/run.sh [--debug|--release]`
  (logs to `tmp/run.log`, sets `LD_LIBRARY_PATH` on `qt-plus/bin/`).
- Headless smoke test (non-blocking, for agents): `./scripts/linux/smoke.sh
  [--debug|--release] [--wait SECONDS]` (default 8s, max 15s). Starts the
  binary offscreen via `timeout`, success = still alive at the end. NEVER run
  the GUI directly from an agent shell: it never exits.

## Tool Execution Policy
- `tmp/` at the repo root is the folder for temporary/scratch files. It is
  gitignored; scripts must create it (`mkdir -p`) if missing. There is no
  other scratch folder (`temp/` is obsolete).
- Never write scratch/generated artifacts into the repo without asking;
  keep the tree clean (`build-debug/`, `build-release/`, `tmp/` are gitignored).
- `sudo` prompts for a password on this machine: never assume elevation works;
  ask the user to run installs themselves or explicitly approve.
- Run builds sequentially, not in parallel.
- Shell tool timeout: 15 seconds maximum. Long builds go through the scripts
  (which parallelize with `make -j`); never wait on a GUI process.

## Documentation
- Build setup: `README.md`
