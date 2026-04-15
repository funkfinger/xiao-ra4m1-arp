# Story 001: Repo Scaffolded

**As a** developer
**I want** the repository initialised with directory structure, licensing, CI, and baseline docs
**So that** subsequent work lands on an enforced, documented foundation

## Acceptance Criteria
- [x] `firmware/arp/`, `hardware/`, `docs/` directories exist at the repo root
- [x] `hardware/README.md` placeholder explains the directory is populated after breadboard validation (see decisions.md §4)
- [x] Three `LICENSE-*` files at repo root: `LICENSE-firmware` (MIT), `LICENSE-hardware` (CERN-OHL-S v2), `LICENSE-docs` (CC-BY-SA 4.0). Root `README.md` points at each.
- [x] `.gitignore` covers PlatformIO (`.pio/`, `.pioenvs/`, `.piolibdeps/`), KiCad backups (`*.bak`, `*-backups/`), and macOS metadata (`.DS_Store`)
- [x] `.github/workflows/ci.yml` triggers on push and pull_request; job passes on an empty `firmware/arp/` (vacuous green until PR 2)
- [x] `README.md` at repo root: one-paragraph project summary, links to `docs/generative-arp-module.md` and `docs/decisions.md`, current hardware rev (none yet), license summary
- [x] `CHANGELOG.md` at repo root exists and has an `## [Unreleased]` section documenting this scaffolding work (decisions.md §11)
- [x] Initial commit pushed to `main` on `github.com/funkfinger/xiao-ra4m1-arp` with CI green

## Notes
- No firmware code yet. `firmware/arp/` is empty or contains a placeholder `README.md` only.
- CI is scaffolded intentionally early so later PRs don't have to also set it up.
- The repo may be renamed to `xiao-ra4m1-module` later — do not hardcode the current name anywhere it would be painful to change.

## Status
done (initial commit)
