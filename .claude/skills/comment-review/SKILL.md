---
name: comment-review
description: Check the owner's code comments in this clox project — English grammar and comment format first, then whether each claim matches the code. Use when the owner asks about a comment they wrote, pastes one, or says a comment is ready; they often phrase it in Japanese ("コメント見て", "このコメントどう?", "書いたよ").
---

# Reviewing the owner's code comments

The owner writes every comment in `ex01/` by hand and is not confident in
English. This skill checks a comment cheaply first (language and format), then
checks its content against the code. **Never edit the C files**: the owner
pastes the fixes in themselves.

## 1. Find the comments to check

In this order:

1. A comment the owner pasted into the message.
2. A file, struct or function the owner named: read the comments on it.
3. Otherwise, the uncommitted changes: `git diff -- 'ex01/*.c' 'ex01/*.h'`,
   plus `git diff --cached` for the same paths. Check only added or changed
   comment lines.

If nothing turns up (empty diff, nothing named), say so in one line and ask
which comment to look at. If the owner mentions a comment that is not in the
diff, say it looks unsaved.

## 2. Pass 1: grammar and format (cheap)

Read only the comment text and the few lines of code it is attached to. Do not
open other files for this pass.

- **Grammar:** spelling, articles (a/the), singular/plural, verb agreement,
  word choice that changes the meaning, and words that are hard to read in
  English (such as "dyn array"). Leave wording alone when it is correct but not
  how you would say it.
- **Format:** the house style below. Also alignment of trailing `///<`
  comments, stray empty lines inside a `/** */` block, and inconsistent terms
  within one comment block. There is no `.clang-format`, so alignment is fixed
  by hand.

## 3. Pass 2: content

Now read the code the comment describes, and whatever it depends on. Check
every claim against **this repository's code**, not the book or memory; later
chapters changed earlier code (for example, Chapter 26 added the GC push/pop in
`addConstant`).

- A claim that is wrong or incomplete: say what the code does, with
  `file:line`.
- A claim that is right but surprising: confirm it briefly, with `file:line`.
- A comment that only restates the code: say so, and point to the
  non-obvious *why* nearby, if there is one.

## 4. Output

Always this layout, in this order, in plain English. Each item is one line
where possible. A section with nothing to report says `Nothing to fix.`

```
**Checked:** <file> — <what> (from <diff | named | pasted>)

### Grammar
1. `<before>` → `<after>` — <short reason>

### Format
1. <line or field> — <what to change>

### Content
1. ✗ <claim> — <what the code does> (`file:line`)
2. ✓ <claim> — <confirmed> (`file:line`)

### Ready to paste
<the whole comment block, with every fix above applied>
```

- Content: list ✗ items first, ✓ items after.
- Quote only the words that change in Grammar, not the whole sentence.
- Keep the reasons short; no paragraphs.
- End with one line: what the owner should do next (for example, "Save
  `object.h` and ask again for `ObjFunction`.").

## House style

The format check follows this list. Add a rule only after the owner agrees to
it, and say in the reply that you added it.

- Types and functions get a Doxygen block: `/** @brief <one line>. ... */`.
- Struct fields get a trailing `///<` comment; a continuation line starts with
  `///<` lined up under the first one.
