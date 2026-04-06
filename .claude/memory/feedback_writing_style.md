---
name: Blog writing style decisions
description: Tone, grammar, and formatting conventions for goose.sh blog posts
type: feedback
---

Use formal tone across all posts — no contractions (write "I am", "it is", "does not", not "I'm", "it's", "doesn't").

**Why:** User wants consistent, polished reading experience across the entire series.
**How to apply:** When writing or editing any blog post.

Arduino board names must be ALL-CAPS: MEGA, DUE, GIGA, UNO. Not "Mega", "Due", etc.

**Why:** Matches official Arduino branding and the convention established in later posts.
**How to apply:** In both prose and frontmatter (description/summary fields).

Every post should open with a `## TLDR` section before any other content (including `## Motivation`).

**Why:** Gives every post a consistent entry point; lets returning readers quickly assess relevance.
**How to apply:** When creating new posts, always start with `## TLDR`.

Do not add per-post preambles (project mission statements, bullet lists of learning goals, links to prior posts). Keep the opening clean — go straight to `## TLDR`.

**Why:** Series context belongs on the project page, not repeated in individual posts. Keeps openings consistent.
**How to apply:** When writing new posts or reviewing existing ones.

Use "bits" vs "bytes" vs "Kbit" precisely — do not write "64 bits" when meaning "64 bytes", or "256K" when meaning "256 Kbit".
