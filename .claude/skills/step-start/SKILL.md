---
name: step-start
description: Start the next phase of this clox (Crafting Interpreters) project. Reads the open/closed GitHub issues to find where the work stands, writes the phase spec with comprehension checks, and files it as an English issue. The user writes every line of C. Use when the user asks to begin, resume, or move on to the next phase; the user usually phrases this in Japanese ("次のステップ", "Phase 2 を始めて", "続きをやろう").
---

Run one phase of this learning project. **This skill never writes C.** It
produces a specification, the reasoning behind it, verification steps, and
comprehension checks. The owner implements everything in `ex01/src` and
`ex01/include`.

## Source of truth

**This repository has no `docs/roadmap.md`, and that is deliberate.** Progress
lives in GitHub issues:

- Open issue titled `Chapter NN Phase M: ...` — the phase in flight
- Closed issues of the same shape — finished phases, with their review comments
- The **Phase 1 issue of a chapter carries that chapter's phase map**, so the
  plan for the whole chapter is one issue away

Never invent a roadmap file. If you need the chapter's plan, read the Phase 1
issue of that chapter.

## The working contract

Who does what, and why:

- **The owner writes all C.** Every line in `ex01/src/**` and
  `ex01/include/**` is theirs — including the lines you could obviously
  supply. Naming a function, a type, a struct field, or the call site is
  specification; writing the body is implementation, and implementation is the
  study. **Do not edit those files, not even to fix a build break.** Report it
  instead.
- **You own the scaffolding**: `ex01/Makefile`, `ex01/scripts/`,
  `ex01/examples/gc/`, `.github/workflows/`, and the build/test sections of
  `README.md`. Build plumbing is not the study material, and time spent on it
  is time taken from the chapter.
- **You commit only your own scaffolding**, as `chore(NN): ...` with
  `Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>`. The owner commits
  their implementation. **The owner pushes.**
- **Commit messages follow the book's section numbering**, with the issue
  number appended: `feat(26.3): Marking the roots (#NN)`. One commit per book
  subsection, not one per phase.
- **Everything on GitHub is English** (issue bodies, comments, PR text, commit
  messages) — see the `github-english` skill. Conversation in chat is
  Japanese.
- **Branch**: one branch per chapter (`section/26-garbage-collection`), pushed
  at the end of each phase, one PR at the end of the chapter.

## Before starting

1. `gh issue list --state open`. If the previous phase's issue is still open,
   **do not start a new phase** — push for review and closure of that one
   first.
2. `git status`. A dirty tree means the previous phase has leftovers. Clear
   them first.
3. **Read the owner's code from the previous phase** (`git log --oneline`, then
   `git show`). The spec must name *their* functions and *their* call sites,
   not the book's. Chapters diverge from the book here — the debug macros are
   build flags rather than `#define`s in `common.h`, for instance.
4. Read the chapter's Phase 1 issue for the phase map.

## Writing the spec

**Look up every fact before writing.** Read the actual source, run the actual
build. Never state a bytecode shape, a struct layout, or a function signature
from memory or from the book — this implementation has diverged from both.

The spec contains:

- **Goal** — one sentence, stated as something observable
- **Why** — the background and the reasoning behind the design choices. This is
  the substance of the learning, and what separates a spec from a checklist
- **Files to touch, and what goes in each** — filenames, the functions to add,
  their signatures, where they are called from, the constraints to satisfy.
  **Never the bodies.**
- **Verification** — the exact commands and the output to expect
- **Comprehension checks** — 2 to 3 questions, answers withheld. Every question
  must be answerable from the owner's own code or from a log they can produce
  (`make gc-run`), never from recall of the book
- **A suggested commit message**

## Filing the issue

1. Write the full spec to a temporary file, in English
2. `gh issue create --title "Chapter NN Phase M: <name>" --body-file <file>`
3. Report the issue number and URL
4. **Also print the full spec in chat.** The issue is the record; chat is where
   the work happens

## Answering comprehension checks

The owner answers **in Japanese, in chat**. Grade there, in Japanese. Once an
answer is settled, post the confirmed understanding to the issue as an English
comment — the wrong turns are worth one line, not a transcript.

When the owner is stuck, **do not jump to the answer**. Graded hints: how to
read the relevant log first, then how to narrow it down, then the answer.

## Chapter 26 — Garbage Collection (current)

Phase map, as filed in the Phase 1 issue:

| Phase | Sections | Content | Done when |
|---|---|---|---|
| 1 | 26.1–26.2 | GC skeleton: `collectGarbage()` called from `reallocate()`, logging only | `make gc-run` prints `-- gc begin/end`; `make test` and `make tests` unchanged |
| 2 | 26.3–26.4 | `markRoots`, `markValue`, `markObject`, `markCompilerRoots`, the gray stack, `blackenObject` | mark/blacken lines appear in the log; nothing is freed yet |
| 3 | 26.5 + 26.7 | `sweep()`, `tableRemoveWhite()`, then the three rooting bugs | `make gc-tests` passes |
| 4 | 26.6 | `bytesAllocated`, `nextGC`, `GC_HEAP_GROW_FACTOR` | passes with stress off; the heap grows on its own |

Chapter-26 specifics that shape the specs:

- **The GC bugs in 26.7 are not spelled out.** `ex01/examples/gc/` already
  contains programs that hit all three sites (26.7.1 constant table, 26.7.2
  interning, 26.7.3 concatenation). The Phase 3 spec says "`make gc-tests`
  will fail, and the cause is in the moment `reallocate()` calls the
  collector" — and stops there. Graded hints only.
- **Challenge 1** (the size of the `Obj` header once `isMarked` lands) belongs
  to Phase 2's comprehension checks; **Challenge 2** (avoiding the
  `isMarked` clear in the sweep) to Phase 3's. Challenge 3 (a different
  collector) is out of scope.
- **Debug macros are build flags, not `#define`s.** `DEBUG_STRESS_GC` comes
  from `GC_FLAGS`, `DEBUG_LOG_GC` from `GCLOG_FLAGS`; `make gc-tests` builds
  with stress but **without** logging, because `scripts/test_examples.sh`
  compares stdout and GC log lines would not be filtered out.
- `make tests` runs `examples/` only; `make gc-tests` runs `examples/` plus
  `examples/gc/` against the stress build.

## Closing out

Once the owner implements, verifies, commits, and pushes, `step-review` takes
over. End the spec by saying so.
