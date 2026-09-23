---
name: step-review
description: Review a finished phase of this clox (Crafting Interpreters) project. Pins the review to pushed commit hashes, reads and runs the actual code, and records findings as an English comment on the phase's GitHub issue. Use when the owner asks for a review or reports that a phase is done; they usually phrase it in Japanese ("レビューして", "終わったよ", "確認して").
---

Review one phase of this learning project. You report findings; the owner
fixes them.

## The working contract

**The contract lives in `step-start`.** Read its *The working contract*
section rather than a copy here. The three parts that bind a review:

- **You do not edit `ex01/src/**` or `ex01/include/**`.** A finding states
  what is wrong and which direction the fix goes; it never contains the fix,
  not even to demonstrate. `ex01/test/`, `ex01/examples/`, `ex01/Makefile`,
  `ex01/scripts/` and CI are yours, and you commit and push those yourself as
  `chore(NN): ...`.
- **Never let the owner amend.** Fixes go in new commits; a wrong fix is
  undone with `git revert`. Amending orphans the hash this comment points at.
- **GitHub is English, chat is Japanese.** Progress lives in issues; there is
  no `docs/roadmap.md` and none should be created. Closing the issue is what
  marks a phase done.

## Before reviewing

1. `git status`. **If anything is uncommitted, do not start.** A review is
   pinned to a hash. Ask the owner to commit first.
2. `gh issue list --state open` — find the issue this work belongs to.
3. Pin the target: `git log --oneline` to select this phase's commits. Express
   a range as `base..head`, and say which commits in it are the owner's and
   which are your scaffolding.
4. Check the commits are pushed (`git fetch`, then compare `git rev-parse HEAD`
   against `origin/section/...`). An unpushed hash does not resolve as a link
   from the issue comment.

## Read the actual thing, and run it

**Never review from assumption.** Read the commits with `git show` and read
the files directly. Run everything the issue's Verification section lists, plus
whatever settles a question. Read-only and build commands are yours to run.

**Rebuild before trusting an exit code.** A stale binary in `gc/bin/` once
produced exit 139 for programs that pass after `make clean`; the report would
have been wrong.

**Reproduce every claim in a copy of `ex01` under the scratchpad**, never in
the repository. Implement the fix there to confirm it works, revert it to
confirm the failure, flip a constant, add instrumentation. This is what turns
"this could re-enter the collector" into "2 of 6032 collections land in this
window, 0 under the threshold policy". Chapter 26 produced three findings that
existed only because they were measured:

- A counter and an `fprintf` around a suspect window, to count how often it is
  actually entered.
- Flipping one constant, when a hazard is latent: `GC_HEAP_GROW_FACTOR` 2 → 1
  turned an unreachable path into 22 / 26 and a segfault.
- Writing a Lox program to disprove a guess. "`vm.openUpvalues` looks
  redundant" was wrong, and the program that showed it is four lines.

Never write a finding that rests on "this is probably how it turned out." If a
measurement fails to reproduce, say so plainly rather than keeping the tidier
story.

## A green test suite proves very little here

Chapter 26 produced five defects that every test target passed:

| defect | `make gc-tests` | what actually held |
|---|---|---|
| `pop()` stranded after `return` | 25 / 25 | value stack never returned to base; segfault past `STACK_MAX` |
| two roots deleted from `markRoots()` | 25 / 25 | one truly redundant, one a gap in the suite |
| a "fix" that only suppressed collection | 25 / 25 | collections dropped 49 → 32 |
| allocation total moved inside the branch | 26 / 26 | 1335-deep recursion under `make heap-log` |
| threshold check outside the branch | 26 / 26 | collector reachable from inside `sweep()` |

Each was found by a log, a debugger, a purpose-written program, or a
one-constant experiment — never by the suite. **Plan the review around the
instruments, not around the exit codes.** Useful ones:

- Log invariants checked programmatically over a whole run, rather than by eye.
- Nesting depth: `awk '/^-- gc begin/{d++; if(d>1) n++} /^-- gc end/{d--} END{print n+0}'`.
- The value stack depth at a breakpoint, when a push/pop pair is in question.
- Comparing a number that *must not change* across a fix against one that may.

## Writing findings

Three tiers:

- **Must fix** — real harm. It causes a crash, a leak, a silently wrong
  collection, or it breaks a later phase. **State the concrete failure
  scenario** and the evidence: which program, which allocation, what the
  measured numbers were before and after.
- **Your call** — not wrong, but the owner should be able to say why they
  chose it. Give both sides.
- **Minor** — taste. Say explicitly that it need not be fixed.

**Always state what was done right**, backed by something you verified — not
praise, but because naming what worked is part of the review's job.

Never repeat a finding. If something raised last time is still unfixed, one
line as a leftover.

## Grading the comprehension checks

The owner answers in Japanese in chat; grade there. A wrong answer is not a
finding — it is the point of the exercise. Follow `step-start`'s *Answering
comprehension checks*: hints first, the answer the moment it is asked for, one
numbered step per message when the owner says they are lost.

When the settled understanding is reached, fold a short English summary into
the review comment under `## Comprehension checks`. Include the numbers that
settled it; the transcript is not worth reproducing, the measurements are.

## What to look at in chapter 27

Per phase:

| Phase | What must hold |
|---|---|
| 1 | `class Foo {}` compiles and runs; `make test` covers `OP_CLASS`; `make tests` / `make gc-tests` unchanged |
| 2 | fields can be set and read back; `make test` covers the property opcodes; `make gc-tests` green |

Chapter-27 things worth checking, because they fail silently:

- **The new `ObjType`s in the collector.** `blackenObject()` must reach
  `ObjClass->name`, and `ObjInstance->klass` *plus* its `fields` table via
  `markTable()`. `freeObject()` must free the fields table with `freeTable()`
  before freeing the instance. A missing `case` will not compile under
  `-Wswitch -Werror`; a missing *mark* inside a case compiles and fails only
  on programs that allocate at the wrong moment.
- **New unrooted windows.** `newClass()`, `newInstance()`, and the fields
  table growing inside `OP_SET_PROPERTY` are all allocations. Anything holding
  a fresh object only in a C local across one of them is the chapter-26 bug
  class again. Check the fix roots the object rather than avoiding the
  collection.
- **The property opcodes' stack discipline.** `OP_GET_PROPERTY` pops the
  instance and pushes the value; `OP_SET_PROPERTY` has a value above an
  instance and must leave only the value. An off-by-one here shows up as a
  wrong `make test` case rather than a crash, which is why the tests land
  first.
- **Carried from chapter 26, still true:** the gray stack must not go through
  `reallocate()`; `vm.strings` is weak and `tableRemoveWhite()` runs before the
  sweep; `bytesAllocated` is adjusted in both directions and returns to 0 after
  `freeVM()`; `nextGC` is recomputed after each collection.

## Recording

1. **Open the comment with the reviewed hash**: `Reviewed: abc1234` or
   `abc1234..def5678`, then list which commits are the owner's and which are
   scaffolding
2. `gh issue comment <number> --body-file <file>`
3. Print the same content in chat

Post the comment once the comprehension checks have settled, so the checks go
in with it. Give the findings in chat immediately, though — the owner should
not wait on the checks to start fixing.

## Closing

- **Do not close while any must-fix finding remains.** The owner stacks fix
  commits; verify each and say so.
- Once everything is resolved, post a closing comment and
  `gh issue close <number>`. The closing comment carries what the next chapter
  inherits — the invariants that still bind, and the places where this
  chapter's work will be tested again.
- The chapter's PR comes after its last phase closes — one PR per chapter, you
  open it, the owner merges.
