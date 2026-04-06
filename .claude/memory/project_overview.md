---
name: Content repo overview
description: Hugo content submodule for goose.sh — structure, post series, and relationship to parent repo
type: project
---

This repo is the **content submodule** for the Hugo site goose.sh (inn-goose.github.io). It contains only Markdown and images — no Hugo config or theme. The parent repo is at `../inn-goose.github.io`.

The site uses the **Blowfish** Hugo theme.

## Content sections

- `blog/` — main posts (leaf bundles: `index.md` + `images/`)
- `projects/` — project overview pages
- `about/` — author bio (Ilia, works at Meta on AI ASICs, personal microelectronics blog)
- `tools/` — tools/equipment page

## Post series (as of 2026-04)

**EEPROM Programmer** (8 posts, 2025-08 to 2025-12): Building an Arduino-based EEPROM programmer for AT28C64/AT28C256. Covers R/W implementation, oscilloscope performance measurement, debugging corrupted reads, Serial JSON-RPC API, data corruption from Arduino reset, data polling vs RDY/BUSY, page write, and supported chips.

**DAC SPI** (2 posts, 2026-01): Digital audio project with AD1860 DAC. Covers positive/negative power supply design and AD1860 wiring.

**Experiments** (2 posts, 2025-09 to 2025-10): Standalone Arduino experiments — pin toggle performance analysis and debugging misconfigured pins.

**Projects** (2 posts): Overview pages for the EEPROM Programmer project and the Serial JSON-RPC Arduino library.

## Related repos

- `inn-goose/inn-goose.github.io` — parent Hugo site
- `inn-goose/eeprom-programmer` — EEPROM programmer firmware/code
- `inn-goose/serial-json-rpc-arduino` — Serial JSON-RPC library

**Why:** Understanding the repo structure and post series helps provide contextual suggestions when writing or editing posts.
**How to apply:** When creating new posts, follow the existing series naming conventions (e.g., `eeprom-programmer-N-slug/`, `dac-spi-N-slug/`). Maintain consistent frontmatter format with `###` separators.
