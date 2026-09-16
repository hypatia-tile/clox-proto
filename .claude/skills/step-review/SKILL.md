---
name: step-review
description: Review a finished phase of this clox (Crafting Interpreters) project. Pins the review to pushed commit hashes, reads the actual code, and records findings as an English comment on the phase's GitHub issue. Never writes C. Use when the user asks for a review or reports that a phase is done; the user usually phrases this in Japanese ("レビューして", "終わったよ", "確認して").
---

Review one phase of this learning project. **This skill never writes C.** It
reports findings; the owner fixes them.

## The working contract

Same contract as `step-start`, and the half that matters here:

- **The owner writes all C.** A finding states what is wrong and which
  direction the fix goes. **It never contains the fix.** Do not edit
  `ex01/src/**` or `ex01/include/**`, not even to demonstrate.
- **Never let the owner amend.** Fixes go in new commits — amending orphans
  the hash this review comment points at.
- **Everything on GitHub is English.** The conversation about the findings is
  Japanese, in chat.
- Progress lives in GitHub issues; **there is no `docs/roadmap.md`** and none
  should be created. Closing the issue is what marks a phase done.

## Before reviewing

1. `git status`. **If anything is uncommitted, do not start.** A review is
   pinned to a hash. Ask the owner to commit first.
2. `gh issue list --state open` — find the issue this work belongs to.
3. Pin the target: `git log --oneline` to select this phase's commits (there
   will usually be several, one per book subsection). Express a range as
   `base..head`.
4. Check the commits are pushed (`git status -sb`, or compare against
   `origin/section/...`). An unpushed hash does not resolve as a link from the
   issue comment.

## Read the actual thing

**Never review from assumption.** Read the commits with `git show` and read the
files directly. Run what can be run: `make test`, `make tests`, `make gc-tests`,
and `make gc-run inputfile=...` when a log would settle a question. Read-only
and build commands are yours to run.

Never write a finding that rests on "this is probably how it turned out."

## Writing findings

Three tiers:

- **Must fix** — real harm. It causes a crash, a leak, or a silently wrong
  collection, or it breaks a later phase. **State the concrete failure
  scenario**: which program, which allocation, what gets freed while still
  reachable.
- **Your call** — not wrong, but the owner should be able to say why they chose
  it. Give both sides.
- **Minor** — taste. Say explicitly that it need not be fixed.

**Always state what was done right**, backed by something you verified — not
praise, but because naming what worked is part of the review's job.

Never repeat a finding. If something raised last time is still unfixed, one
line as a leftover.

## Grading the comprehension checks

The owner answers in Japanese in chat; grade there. A wrong answer is not a
finding — it is the point of the exercise. When the settled understanding is
reached, fold a short English summary into the review comment, under
`## Comprehension checks`.

## What to look at in chapter 26

Verification per phase:

| Phase | What must hold |
|---|---|
| 1 | `make gc-run` prints `-- gc begin/end`; `make test` / `make tests` unchanged |
| 2 | mark and blacken lines in the log; still nothing freed; `make gc-tests` still passes |
| 3 | `make gc-tests` passes — including `examples/gc/` |
| 4 | passes with stress off; `nextGC` grows across collections |

GC-specific things worth checking, because they fail silently:

- **Root coverage** — the value stack, every `CallFrame`'s closure, the open
  upvalue list, the globals table, and the compiler's function chain. A missing
  root only bites on programs that allocate at the wrong moment.
- **The gray stack must not go through `reallocate()`** — growing it inside a
  collection would re-enter the collector. The book uses bare `realloc` on
  purpose.
- **`vm.strings` is a weak table** and must be swept with `tableRemoveWhite`
  *before* the sweep frees the strings, or the table keeps dangling keys.
- **`bytesAllocated` must be adjusted in `reallocate()` for both directions**,
  and `nextGC` recomputed after each collection.
- **Interned strings and constants** must be reachable while being installed —
  this is what 26.7 is about; a review after Phase 3 should confirm the fix is
  rooting (push/pop), not a workaround that disables collection.

## Recording

1. **Open the comment with the reviewed hash**: `Reviewed: abc1234` or
   `abc1234..def5678`
2. `gh issue comment <number> --body-file <file>`
3. Print the same content in chat

## Closing

- **Do not close while any must-fix finding remains.** The owner stacks fix
  commits; add a follow-up comment naming those hashes.
- Once everything is resolved, post a closing comment and
  `gh issue close <number>`.
- The chapter's PR comes after its last phase closes — one PR per chapter.
