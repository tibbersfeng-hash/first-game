# Asset Manifest

> Last updated: 2026-06-16

## Progress Summary

| Total | Needed | Specs Done | Generated | Done |
|-------|--------|------------|-----------|------|
| 50 | 50 | 36 | 233 frames (78+78+77) | 0 |

## Assets by Context

### Character: 慧空 (Blazing Monkfist)
| Asset ID | Name | Category | Status | Spec File |
|----------|------|----------|--------|-----------|
| ASSET-001 | Idle (8帧) | Sprite | **Generated** | specs/huikong-assets.md |
| ASSET-002 | Walk (8帧) | Sprite | **Generated** | specs/huikong-assets.md |
| ASSET-003 | Run (6帧) | Sprite | **Generated** | specs/huikong-assets.md |
| ASSET-004 | Light Attack ×3 (16帧) | Sprite | **Generated** | specs/huikong-assets.md |
| ASSET-005 | Heavy Attack (8帧) | Sprite | **Generated** | specs/huikong-assets.md |
| ASSET-006 | Special 必杀 (16帧) | Sprite | **Generated** | specs/huikong-assets.md |
| ASSET-007 | Dodge (5帧) | Sprite | **Generated** | specs/huikong-assets.md |
| ASSET-008 | Hit (4帧) | Sprite | **Generated** | specs/huikong-assets.md |
| ASSET-009 | Jump (4帧) | Sprite | **Generated** | specs/huikong-assets.md |
| ASSET-010 | Portrait/Avatar | UI | **Generated** | specs/huikong-assets.md |
| ASSET-011 | Skill Icon: 气功波 | UI | **Generated** | specs/huikong-assets.md |
| ASSET-012 | Skill Icon: 如来神掌 | UI | **Generated** | specs/huikong-assets.md |

### Character: 糖糖 (Sugar Hexblade)
| Asset ID | Name | Category | Status | Spec File |
|----------|------|----------|--------|-----------|
| ASSET-013 | Idle (8帧) | Sprite | **Generated** | specs/tangtang-assets.md |
| ASSET-014 | Walk (8帧) | Sprite | **Generated** | specs/tangtang-assets.md |
| ASSET-015 | Run (6帧) | Sprite | **Generated** | specs/tangtang-assets.md |
| ASSET-016 | Light Attack ×3 (16帧) | Sprite | **Generated** | specs/tangtang-assets.md |
| ASSET-017 | Heavy Attack (8帧) | Sprite | **Generated** | specs/tangtang-assets.md |
| ASSET-018 | Special 必杀 (16帧) | Sprite | **Generated** | specs/tangtang-assets.md |
| ASSET-019 | Dodge (5帧) | Sprite | **Generated** | specs/tangtang-assets.md |
| ASSET-020 | Hit (4帧) | Sprite | **Generated** | specs/tangtang-assets.md |
| ASSET-021 | Jump (4帧) | Sprite | **Generated** | specs/tangtang-assets.md |
| ASSET-022 | Portrait/Avatar | UI | **Generated** | specs/tangtang-assets.md |
| ASSET-023 | Skill Icon: 糖果地雷 | UI | **Generated** | specs/tangtang-assets.md |
| ASSET-024 | Skill Icon: 糖果风暴 | UI | **Generated** | specs/tangtang-assets.md |

### Character: 小鬼丸 (Oni Crushkin)
| Asset ID | Name | Category | Status | Spec File |
|----------|------|----------|--------|-----------|
| ASSET-025 | Idle (8帧) | Sprite | **Generated** | specs/kiguemaru-assets.md |
| ASSET-026 | Walk (8帧) | Sprite | **Generated** | specs/kiguemaru-assets.md |
| ASSET-027 | Run (6帧) | Sprite | **Generated** | specs/kiguemaru-assets.md |
| ASSET-028 | Light Attack ×3 (18帧) | Sprite | **Generated** | specs/kiguemaru-assets.md |
| ASSET-029 | Heavy Attack (8/10帧) | Sprite | **Generated (8/10)** | specs/kiguemaru-assets.md |
| ASSET-030 | Special 必杀 (16/20帧) | Sprite | **Generated (16/20)** | specs/kiguemaru-assets.md |
| ASSET-031 | Dodge (5帧) | Sprite | **Generated** | specs/kiguemaru-assets.md |
| ASSET-032 | Hit (3帧) | Sprite | **Generated** | specs/kiguemaru-assets.md |
| ASSET-033 | Jump (4帧) | Sprite | **Generated** | specs/kiguemaru-assets.md |
| ASSET-034 | Portrait/Avatar | UI | **Generated** | specs/kiguemaru-assets.md |
| ASSET-035 | Skill Icon: 跳砸 | UI | **Generated** | specs/kiguemaru-assets.md |
| ASSET-036 | Skill Icon: 鬼王降临 | UI | **Generated** | specs/kiguemaru-assets.md |

### Output Directory

```
design/assets/output/
├── huikong/     (78 frames, 16MB)
├── tangtang/    (78 frames, 29MB)
── kiguemaru/   (77 frames, 28MB)
└── test/        (debug images)
```

### Generation Tool

- Script: `tools/generate_sprites.py`
- Model: wanx2.1-t2i-turbo (via bailian CLI)
- Strategy: Multi-batch × 4 images per batch, seed variation

### Remaining (not yet specced)
| Range | Items | Description |
|-------|-------|-------------|
| ASSET-037~050 | 14项 | Enemies, Bosses, VFX, Environment, UI Screens, HUD, Audio |

### Post-Processing TODO
- [ ] Remove backgrounds from generated frames (replace with transparency)
- [ ] Resize frames to 256×256px
- [ ] Assemble sprite sheets (horizontal strip format)
- [ ] Manual curation: pick most consistent frames per animation
- [ ] Import into Godot project as AnimatedSprite2D resources
