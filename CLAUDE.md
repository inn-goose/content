# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What This Repo Is

This is the **content submodule** for the Hugo site at [inn-goose.github.io](https://github.com/inn-goose/inn-goose.github.io). It contains only Markdown posts and images — no Hugo config, theme, or build system. The parent repo lives at `../inn-goose.github.io` and references this as a git submodule under `content/`.

The site is a personal microelectronics blog (goose.sh) documenting hands-on experiments with EEPROMs, Arduinos, DACs, oscilloscopes, and related hardware.

## Building / Previewing

Since this repo has no Hugo config, build from the **parent repo**:

```bash
cd ../inn-goose.github.io
hugo server -D          # dev server with drafts at localhost:1313
hugo --minify --gc      # production build
```

## Content Structure

Posts use **Hugo leaf bundles** — each post is a directory containing `index.md` plus images:

```
blog/eeprom-programmer-1-initial-implementation/
├── index.md           # post content
├── featured.jpeg      # featured/hero image
└── images/            # post-specific images
```

Section listing pages use `_index.md` (e.g., `blog/_index.md`, `projects/_index.md`).

## Frontmatter Format

Posts use YAML frontmatter with `###` comment separators between fields:

```yaml
---
date: 2025-08-28
###
title: "Post Title"
###
description: "Short description"
summary: "Longer summary for listings"
###
tags: [tag1, tag2]
---
```

## Key Conventions

- **Images**: Store in `images/` subdirectory within the post bundle. Raw/source images go in `RAW/` (gitignored).
- **Links**: Use standard single-paren markdown links `[text](url)`. The Blowfish theme rejects double-paren links `[text]((url))` — these will break the Hugo build.
- **Internal links**: Use root-relative paths like `/blog/post-slug/` (not `../` relative paths).
