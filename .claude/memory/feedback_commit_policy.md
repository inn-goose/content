---
name: Commit policy for memory files
description: Never bundle .claude/memory changes into commits with other work — only commit when explicitly asked
type: feedback
---

Never commit `.claude/memory/` files together with other changes. Only commit memory/context files when explicitly asked to do so.

**Why:** Memory updates are housekeeping, not part of the actual work. Bundling them pollutes the git history.
**How to apply:** When committing changes, exclude `.claude/memory/` files unless the user explicitly says to commit them.
