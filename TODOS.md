# Persistent Todos

Use this file to keep track of ongoing work across agent sessions.

## Current Tasks
- [x] Review the current repository state and identify next actions
- [x] Document any blockers or follow-up questions
- [x] Update this file as progress changes
- [ ] Investigate and report the bugs in the Ubuntu/Debian packages for libqpid-proton-dev

## Completed Context
- Built the repository successfully after adding a fallback for Proton using `pkg-config` when the `Proton` CMake package is not available.
- Verified the build and runtime of `build/amqp-value-test` in the current environment.
- Confirmed that `protocol.h.py` is a protocol header generator from the AMQP XML files and is not used to generate `performatives.gperf`.
- The current package-level issue is that Debian/Ubuntu `libqpid-proton-dev` provides `pkg-config` metadata but does not export a usable `Proton` CMake package.
- Further issue is that the `pkg-config` metadata has an incorrect path for include and library includes.
- Another issue is that there is only a .pc file for libqpid-proton but there should be .pc files for libqpid-proton-core and libqpid-proton-proactor too.

## Notes
- Keep tasks short and concrete.
- Mark items as completed once finished.
- Add new tasks here instead of relying on transient chat context.
- When starting any new work item, add it to this file first so it remains persistent across sessions.
