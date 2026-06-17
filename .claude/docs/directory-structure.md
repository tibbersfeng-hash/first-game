# Directory Structure

```text
/
├── CLAUDE.md                    # Master configuration
├── .claude/                     # Agent definitions, skills, hooks, rules, docs
├── src_ue5/                     # UE5 C++ source (FirstGame module)
│   ├── FirstGame.uproject       # UE5 project file
│   ├── Config/                  # UE5 config (ini files)
│   ├── Content/                 # UE5 assets (Blueprints, meshes, textures)
│   └── Source/FirstGame/        # C++ source (Public/ + Private/)
├── design/                      # Game design documents (gdd, narrative, levels, balance)
│   └── prototypes/              # Archived prototype design docs (e.g. combat-concept)
├── docs/                        # Technical documentation (architecture, api, postmortems)
│   └── engine-reference/        # Curated engine API snapshots (version-pinned)
├── tools/                       # Build and pipeline tools
│   └── asset-generation/        # AI sprite generation scripts (DashScope)
└── production/                  # Production management (sprints, milestones, releases)
    ├── session-state/           # Ephemeral session state (active.md — gitignored)
    └── session-logs/            # Session audit trail (gitignored)
```

> **Note**: Legacy Godot directories (`src/`, `assets/`, `addons/`, `prototypes/`, `tests/`)
> were removed on 2026-06-17 during the UE5 migration. See HANDOVER.md for Godot-era history.
