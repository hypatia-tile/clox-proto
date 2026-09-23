---
name: step-start
description: Start the next phase of this clox (Crafting Interpreters) project. Reads the GitHub issues to find where the work stands, builds a reference implementation to measure against, writes the phase spec with comprehension checks, lands the bytecode tests, and files it all as an English issue. Use when the owner asks to begin, resume, or move on to the next phase; they usually phrase it in Japanese ("次のステップ", "Phase 2 を始めて", "続きをやろう", "chapter 27 を始めたい").
---

Run one phase of this learning project. You produce a specification, the
reasoning behind it, the bytecode tests that pin it, and comprehension checks.
The owner implements the interpreter.

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

This section is the contract for both skills. `step-review` refers back here
rather than restating it.

- **The owner writes the interpreter.** Every line in `ex01/src/**` and
  `ex01/include/**` is theirs — including the lines you could obviously
  supply. Naming a function, a type, a struct field, or a call site is
  specification; writing the body is implementation, and implementation is the
  study. **Do not edit those two trees, not even to fix a build break.**
  Report it instead.
- **You own everything else**: `ex01/Makefile`, `ex01/scripts/`,
  `ex01/examples/`, `ex01/test/`, `.github/workflows/`, and the build/test
  sections of `README.md`. You write C in `ex01/test/src/` — the bytecode
  tests are yours (see *Bytecode tests* below). Build plumbing is not the
  study material, and time spent on it is time taken from the chapter.
- **You commit and push your own work**, as `chore(NN): ...` with
  `Co-Authored-By: Claude Opus 5 <noreply@anthropic.com>`. The owner commits
  and pushes theirs. Neither of you touches the other's commits. This matters
  now that tests land before the implementation and CI runs on every push: an
  unpushed `chore` commit means the owner cannot start and CI never sees it.
- **Commit messages follow the book's section numbering**, with the issue
  number appended: `feat(27.2): Class declarations (#NN)`. One commit per book
  subsection, and one per site where a phase splits into independent fixes.
- **Never amend.** Fixes go in new commits; a mistaken fix is undone with
  `git revert`, not by rewriting. Review comments are pinned to hashes.
- **Everything on GitHub is English** (issue bodies, comments, PR text, commit
  messages) — see the `github-language` skill. Conversation in chat is
  Japanese; see `conversation-language` for which terms stay English.
- **Branch**: one branch per chapter (`section/27-classes-and-instances`),
  pushed continuously, one PR at the end of the chapter. You file and close
  the issues and open the chapter PR; the owner merges.

## Before starting

1. `gh issue list --state open`. If the previous phase's issue is still open,
   **do not start a new phase** — push for review and closure of that one
   first.
2. `git status`. A dirty tree means the previous phase has leftovers. Clear
   them first.
3. **Read the owner's code from the previous phase** (`git log --oneline`,
   then `git show`). The spec must name *their* functions and *their* call
   sites, not the book's. This implementation has diverged — the debug macros
   are build flags rather than `#define`s in `common.h`, for instance.
4. Read the chapter's Phase 1 issue for the phase map. At the start of a new
   chapter, propose one (see *Splitting a chapter* below).

## Splitting a chapter into phases

A phase is **a unit that can be verified on its own**: after it, some `make`
target returns a meaningful result that it did not return before. That is the
only criterion. It is not one phase per book section — chapter 26 ran
26.1–26.2 / 26.3–26.4 / 26.5+26.7 / 26.6, because those were the points where
something became observable.

Some sections produce nothing verifiable alone (a new `ObjType` that nothing
constructs yet); fold them forward into the phase that first uses them.

Propose the map at the start of a chapter and put it in the Phase 1 issue.

## Build a reference implementation first

**Before writing a word of the spec, implement the phase yourself** in a copy
of `ex01` under the scratchpad directory — never in the repository. Then
measure everything the spec will claim.

This is not optional, and it is not for the code. It is for three things the
spec cannot get right otherwise:

- **The verification numbers.** "26 / 26 passed", "65 collections",
  "collected 995263 bytes (from 1048657 to 53394)". A spec that says "should
  pass" gives the owner nothing to check against.
- **The comprehension checks' answers.** Chapter 26's Phase 2 shipped a check
  asking the owner to find a log line that *did not exist in any input*,
  because nothing cleared the mark bit that phase. A reference implementation
  would have caught it in one run.
- **The tooling the phase needs.** You only discover that a question needs a
  stress-free `-g` build by trying to answer it. Everything the spec asks for
  must exist before the issue is filed, not be improvised mid-phase.

Measure against the repository's own tree, not only the copy: chapter 26's
Phase 3 spec quoted "11 / 25" from a scratch build that did not match what the
owner saw (17 / 25), and the issue had to be corrected.

Keep the reference out of the repository, and out of the spec: it tells you
what is true, not what to write down.

## Bytecode tests

`ex01/test/src/test_compiler.c` is table-driven — each case is a `name`, a
`source` snippet, `BYTES(...)` and `CONSTANTS(...)`. A case pins exactly what
the compiler must emit.

**You write the cases for the phase's new opcodes, and they land with the
issue, before the owner implements.** The spec says how many will fail until
the phase is done. This is the executable half of the specification: chapter
26 lost several exchanges to prose that was read correctly and applied to the
wrong line, and a failing test says which line without ambiguity.

The cases encode the emit order *you* chose in the reference implementation.
If the owner's order differs and is equally correct, **the test is wrong, not
the owner** — fix the test.

A phase that adds no opcode adds no cases, and `make test` stays a pure
regression check.

## Writing the spec

**Look up every fact before writing.** Read the actual source, run the actual
build. Never state a bytecode shape, a struct layout, or a function signature
from memory or from the book.

The spec contains:

- **Goal** — one sentence, stated as something observable
- **Why** — the background and the reasoning behind the design choices. This
  is the substance of the learning, and what separates a spec from a checklist
- **Files to touch, and what goes in each** — filenames, the functions to add,
  their signatures, where they are called from, the constraints to satisfy.
  **Never the bodies.**
- **Verification** — the exact commands and the measured output, including
  which tests fail before the phase is implemented
- **Comprehension checks** — see below
- **Suggested commit messages**, one per book subsection

## Comprehension checks

Two to three per phase. The binding constraint:

> **Every check must be answerable with the `make` targets that already exist.**

If answering needs a build configuration or an example program that is not
there, add it as scaffolding and push it *before* filing the issue. If a check
needs a debugger, give a recipe with the line numbers already filled in —
never a placeholder to substitute, which is how chapter 26's Phase 4 stalled.

Good checks, by what chapter 26 produced:

- **Experiments** are the strongest. "Delete one root at a time from
  `markRoots()`, run `make gc-tests` after each, record all five scores" —
  five commands, and it ends with the owner discovering that two roots survive
  removal for completely different reasons.
- **Measurements that contradict an assumption.** "53 KB survives, the
  program's own live data is 1.4 KB, find the other 52 KB."
- **Reasoning from their own code** needs no tooling at all and can be the
  third question.

The book's challenges are only sometimes usable: chapter 26's first two had
single correct answers and made good checks; chapter 27's four are open-ended
language-design prompts and cannot be graded. Check before planning around
them.

## Filing the issue

1. Push the phase's scaffolding and bytecode tests first, as `chore(NN): ...`
2. Write the full spec to a temporary file, in English
3. `gh issue create --title "Chapter NN Phase M: <name>" --body-file <file>`
4. Edit the body to substitute the real issue number into the suggested
   commit messages
5. Report the issue number and URL
6. **Also print the full spec in chat.** The issue is the record; chat is
   where the work happens

## Answering comprehension checks

The owner answers **in Japanese, in chat**. Grade there, in Japanese. Once an
answer is settled, fold a short English summary into the review comment — the
wrong turns are worth one line, not a transcript.

Start with graded hints: how to read the relevant log, then how to narrow it
down. **But the moment the owner asks for the answer, give it — do not
negotiate.** What chapter 26 showed is that withholding was never the useful
part; the *format* was. So when you give it:

- **One step per message**, ending with an explicit check that it landed,
  before the next step. A complete explanation in one message does not land
  when the owner has said they are lost.
- Build on their partial answers. "でも free では collectGarbage しません" was
  half right, and the half that was right was the way in.
- **If a described edit is misapplied twice, stop describing it.** Show the
  shape with the statements elided — `<閾値判定と collect>` inside the braces —
  which says which line moves without writing their C.

## Chapter 27 — Classes and Instances (current)

Sections: 27.1 Class Objects, 27.2 Class Declarations, 27.3 Instances of
Classes, 27.4 Get and Set Expressions (27.4.1 Interpreting getter and setter
expressions).

Proposed phase map — put the agreed version in the Phase 1 issue:

| Phase | Sections | Content | Done when |
|---|---|---|---|
| 1 | 27.1–27.2 | `ObjClass`, `OP_CLASS`, class declarations | `class Foo {}` compiles and runs; `make test` covers the new opcode |
| 2 | 27.3–27.4 | `ObjInstance` with its fields table, `OP_GET_PROPERTY` / `OP_SET_PROPERTY` | fields can be set and read; `make gc-tests` still green |

Chapter-27 specifics that shape the specs:

- **This is the first chapter since 26 where the bytecode changes.** `make
  test` stops being a pure regression check; see *Bytecode tests*.
- **Two new `ObjType`s arrive, and the collector must learn both.**
  `blackenObject()` and `freeObject()` each need a case — `-Wswitch` under
  `-Werror` will say so — and `ObjClass` reaches its `name` while
  `ObjInstance` reaches its `klass` *and* its `fields` table, which needs
  `markTable()`. A missing case will not compile; a missing *mark* fails
  silently, and only on programs that allocate at the wrong moment.
- **Every new allocation site is a new unrooted window.** `newClass()` and
  `newInstance()` allocate, and so does the fields table when it grows. The
  chapter-26 invariant applies unchanged: a freshly allocated object must not
  be left unreachable from every root across another allocation. `make
  gc-tests` (stress) is the instrument that finds these; it is not optional
  and it does not come off.
- **The book's four challenges are open-ended design prompts** (missing
  fields, computed field names, field deletion, optimising field access).
  None has a single gradeable answer, so comprehension checks for this chapter
  have to be built from the reference implementation instead.
- `make tests` runs `examples/` only; `make gc-tests` runs `examples/` plus
  `examples/gc/` against the stress build. Debug macros are build flags, not
  `#define`s.

## Closing out

Once the owner implements, verifies, commits and pushes, `step-review` takes
over. End the spec by saying so.

**Tell the owner to run the whole Verification section before asking for a
review.** Chapter 26's Phase 2 was pushed with `make gc-tests` at 0 / 25, all
twenty-five cases segfaulting, because the section was never run. CI now runs
on pushes to `section/**` as well as on pull requests, so this is caught either
way — but it is caught faster locally.
