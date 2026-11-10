# Repository Agent Instructions

## Project Context
- Course: Operating Systems
- Source: xv6-labs-2024, RISC-V
- Required features: xargs, tree, tracing, procinfo
- Deadline: 2026-11-11
- Follow Project 1.pdf and the course materials.

## Before Editing
- Read PROJECT.md and inspect the existing implementation.
- Identify the files that need to change before editing.
- Keep changes limited to the assigned task.
- Do not overwrite unrelated files or documentation.
- Ask for clarification when requirements are ambiguous.

## Git Workflow
- Never implement features directly on main.
- Create a separate feature branch for each task.
- Never run git push --mirror or force-push shared branches.
- Do not discard uncommitted changes without permission.
- Review changes before merging into main.
- Never commit credentials, tokens, or unrelated generated files.

## xv6 Development
- Follow existing xv6 C coding conventions.
- Do not edit generated user/usys.S directly.
- Update user/usys.pl when adding system call stubs.
- Coordinate changes to shared syscall files.
- Do not change existing system call numbers without justification.

## Testing
- Build and boot xv6 using make clean && make qemu.
- Test the actual functionality in QEMU.
- Include normal, boundary, and error cases where applicable.
- Report files changed, tests executed, results, and limitations.
- Never claim a test passed unless it was actually run.
