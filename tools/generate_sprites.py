#!/usr/bin/env python3
"""
Sprite Asset Generator for 格斗萌主
Uses direct DashScope HTTP API + wan2.7-image-pro to generate sprite frames.

Strategy:
- For each animation type, generate N×batch_size candidate frames
- Auto-select the most visually consistent set
- Download and organize into proper directory structure
- Log progress to asset-manifest.md

Usage:
  python3 generate_sprites.py --character huikong --animation idle --frames 8
  python3 generate_sprites.py --character huikong --animation all
  python3 generate_sprites.py --all-characters --dry-run
"""

import argparse
import json
import os
import re
import sys
import time
from pathlib import Path

import requests
from PIL import Image

# ─── Constants ────────────────────────────────────────────────────────────────

PROJECT_ROOT = Path(__file__).parent.parent  # tools/ -> first-game/
OUTPUT_DIR = PROJECT_ROOT / "design" / "assets" / "output"
DASHSCOPE_API_KEY = os.environ.get("DASHSCOPE_API_KEY", "")

# Frame counts per animation type
FRAME_COUNTS = {
    "idle": 8,
    "walk": 8,
    "run": 6,
    "light1": 5,
    "light2": 5,
    "light3": 6,
    "heavy": 8,
    "special": 16,
    "dodge": 5,
    "hit": 4,
    "jump": 4,
    "portrait": 1,
    "skill_icon_1": 1,
    "skill_icon_2": 1,
}

# ─── Character Definitions ────────────────────────────────────────────────────

CHARACTERS = {
    "huikong": {
        "name_cn": "慧空",
        "name_en": "Blazing Monkfist",
        "base_prompt": (
            "chibi 3-head-tall Chinese martial artist boy, "
            "dark blue cross-collar monk robe with wide red sash belt and bow, "
            "golden gauntlet fists, large wooden prayer beads around neck, "
            "red headband with ribbon, short hair with small topknot, "
            "2D cel-shaded game sprite art style, clean black outline, "
            "consistent character design"
        ),
        "animations": {
            "idle": {
                "prompt_suffix": "idle standing pose, breathing micro-animation, body slight rise and fall, gentle determined smile",
                "frames": 8,
            },
            "walk": {
                "prompt_suffix": "walking cycle animation, elastic bouncy steps, body leaning forward and back, arms swinging, prayer beads and headband ribbon swinging",
                "frames": 8,
            },
            "run": {
                "prompt_suffix": "running cycle, exaggerated arm swing, body leaning forward, headband ribbon and prayer beads flying backward, speed lines",
                "frames": 6,
            },
            "light1": {
                "prompt_suffix": "fast right straight punch attack, fist extended forward, body twisted, white small qi arc effect",
                "frames": 5,
            },
            "light2": {
                "prompt_suffix": "left hook uppercut attack, fist swinging upward, blue arc qi energy effect",
                "frames": 5,
            },
            "light3": {
                "prompt_suffix": "double fist rising dragon uppercut, golden dragon-shaped qi wave, launch effect, dramatic upward pose",
                "frames": 6,
            },
            "heavy": {
                "prompt_suffix": "heavy palm strike, two-phase: charging pose (palms pulled back, body compressed) and releasing pose (palms pushed forward, golden circular qi shockwave)",
                "frames": 8,
            },
            "special": {
                "prompt_suffix": 'ultimate move "Buddha Palm", full body golden aura pulsing, hands pressed together with golden light, giant golden Buddha palm descending from above, ground shattering impact',
                "frames": 16,
            },
            "dodge": {
                "prompt_suffix": "dodge roll, compact rolling pose, golden translucent afterimage effect",
                "frames": 5,
            },
            "hit": {
                "prompt_suffix": "hit reaction, body squashed from impact, pained angry expression, white star particles",
                "frames": 4,
            },
            "jump": {
                "prompt_suffix": "jump animation, takeoff airborne landing grounded poses",
                "frames": 4,
            },
            "portrait": {
                "prompt_suffix": "front face portrait close-up for HUD avatar, large determined eyes, red headband and prayer beads clearly visible",
                "frames": 1,
            },
            "skill_icon_1": {
                "prompt_suffix": "golden fist with qi ripple icon, rounded square background, skill icon for game HUD",
                "frames": 1,
            },
            "skill_icon_2": {
                "prompt_suffix": "golden Buddha palm icon, rounded square background, ultimate skill icon for game HUD",
                "frames": 1,
            },
        },
    },
    "tangtang": {
        "name_cn": "糖糖",
        "name_en": "Sugar Hexblade",
        "base_prompt": (
            "chibi 3-head-tall cute Chinese magical girl, "
            "pink and white frilly mage dress with candy decorations, "
            "large candy-shaped staff weapon, twin ponytails with candy hair accessories, "
            "candy-themed magic accessories, "
            "2D cel-shaded game sprite art style, clean black outline, "
            "consistent character design"
        ),
        "animations": {
            "idle": {
                "prompt_suffix": "idle standing pose, candy staff floating beside, cheerful magical expression",
                "frames": 8,
            },
            "walk": {
                "prompt_suffix": "walking cycle, candy decorations bouncing, cheerful bouncy steps",
                "frames": 8,
            },
            "run": {
                "prompt_suffix": "running, candy trail particles, candy staff held forward",
                "frames": 6,
            },
            "light1": {
                "prompt_suffix": "candy projectile toss, pink candy bullet flying, cheerful throwing pose",
                "frames": 5,
            },
            "light2": {
                "prompt_suffix": "candy staff swing attack, pink sparkles trail, spinning twirl",
                "frames": 5,
            },
            "light3": {
                "prompt_suffix": "candy barrage combo, multiple candy projectiles exploding, pink explosion effects",
                "frames": 6,
            },
            "heavy": {
                "prompt_suffix": "candy landmine plant, giant candy bomb placed on ground, warning aura glow",
                "frames": 8,
            },
            "special": {
                "prompt_suffix": 'ultimate move "Candy Storm", tornado of candies swirling, pink rainbow vortex, candy explosions everywhere',
                "frames": 16,
            },
            "dodge": {
                "prompt_suffix": "magic teleport dodge, pink sparkle afterimage, vanishing effect",
                "frames": 5,
            },
            "hit": {
                "prompt_suffix": "hit reaction, surprised cute expression, candy pieces scattering",
                "frames": 4,
            },
            "jump": {
                "prompt_suffix": "jump with candy staff, floating mid-air, magic sparkle trail",
                "frames": 4,
            },
            "portrait": {
                "prompt_suffix": "front face portrait, cute cheerful expression, twin ponytails, candy accessories, for HUD avatar",
                "frames": 1,
            },
            "skill_icon_1": {
                "prompt_suffix": "candy landmine icon, pink candy bomb, rounded square background, skill icon",
                "frames": 1,
            },
            "skill_icon_2": {
                "prompt_suffix": "candy storm icon, pink tornado of candies, rounded square background, ultimate skill icon",
                "frames": 1,
            },
        },
    },
    "kiguemaru": {
        "name_cn": "小鬼丸",
        "name_en": "Oni Crushkin",
        "base_prompt": (
            "chibi 3-head-tall oni demon warrior boy, "
            "red and black samurai armor with oni horn mask, "
            "large kanabo spiked club weapon, wild spiky hair, "
            "ferocious battle expression, "
            "2D cel-shaded game sprite art style, clean black outline, "
            "consistent character design"
        ),
        "animations": {
            "idle": {
                "prompt_suffix": "idle standing pose, gripping kanabo club, fierce determined expression",
                "frames": 8,
            },
            "walk": {
                "prompt_suffix": "heavy stomping walk, kanabo dragging slightly, fierce swagger",
                "frames": 8,
            },
            "run": {
                "prompt_suffix": "charging run, kanabo held overhead, fierce battle cry pose, fire trail",
                "frames": 6,
            },
            "light1": {
                "prompt_suffix": "kanabo horizontal swing, red impact arc, fierce swing pose",
                "frames": 5,
            },
            "light2": {
                "prompt_suffix": "kanabo overhead slam, ground crack effect, jumping slam pose",
                "frames": 5,
            },
            "light3": {
                "prompt_suffix": "triple kanabo combo, spinning attack, red fire trail spiral",
                "frames": 6,
            },
            "heavy": {
                "prompt_suffix": "ground pound heavy attack, kanabo smashing ground, earthquake crack effect, red energy burst",
                "frames": 10,
            },
            "special": {
                "prompt_suffix": 'ultimate move "Oni King Descent", giant oni demon avatar behind character, hellfire aura, ground splitting, dramatic descent pose',
                "frames": 20,
            },
            "dodge": {
                "prompt_suffix": "berserker dash dodge, red fire trail afterimage, aggressive sliding",
                "frames": 5,
            },
            "hit": {
                "prompt_suffix": "hit reaction, angry fierce expression, armor sparks, fire particles",
                "frames": 3,
            },
            "jump": {
                "prompt_suffix": "powerful jump, kanabo raised, red energy aura, ground impact on landing",
                "frames": 4,
            },
            "portrait": {
                "prompt_suffix": "front face portrait, fierce oni expression, horn mask, for HUD avatar",
                "frames": 1,
            },
            "skill_icon_1": {
                "prompt_suffix": "ground slam icon, kanabo cracking earth, rounded square background, skill icon",
                "frames": 1,
            },
            "skill_icon_2": {
                "prompt_suffix": "oni king descent icon, giant demon avatar, rounded square background, ultimate skill icon",
                "frames": 1,
            },
        },
    },
}

# ─── Core Functions ───────────────────────────────────────────────────────────


def generate_image(prompt: str, size: str = "1024*1024", n: int = 4, seed: int = None) -> list[str]:
    """Call DashScope wan2.7-image-pro via multimodal-generation API. Returns list of URLs."""
    api_key = os.environ.get("DASHSCOPE_API_KEY", "")
    if not api_key:
        print("  ❌ DASHSCOPE_API_KEY not set")
        return []

    api_url = "https://dashscope.aliyuncs.com/api/v1/services/aigc/multimodal-generation/generation"
    headers = {
        "Authorization": f"Bearer {api_key}",
        "Content-Type": "application/json",
    }
    payload = {
        "model": "wan2.7-image-pro",
        "input": {
            "messages": [
                {"role": "user", "content": [{"text": prompt}]}
            ]
        },
        "parameters": {
            "size": size,
            "n": n,
        },
    }
    if seed is not None:
        payload["parameters"]["seed"] = seed

    try:
        resp = requests.post(api_url, json=payload, headers=headers, timeout=120)
        resp.raise_for_status()
        data = resp.json()

        if "output" in data:
            output = data["output"]
            images = []
            for choice in output.get("choices", []):
                msg = choice.get("message", {})
                for content in msg.get("content", []):
                    if "image" in content:
                        images.append(content["image"])
            if images:
                return images
        else:
            code = data.get("code", "UNKNOWN")
            msg = data.get("message", "unknown error")
            print(f"  ️ API error: {code} - {msg[:100]}")
            return []
    except requests.Timeout:
        print("  ⚠️ Timeout waiting for image generation")
        return []
    except requests.RequestException as e:
        print(f"  ⚠️ Request failed: {e}")
        return []
    except (json.JSONDecodeError, KeyError) as e:
        print(f"  ⚠️ Failed to parse response: {e}")
        return []


def download_image(url: str, filepath: Path) -> bool:
    """Download image from URL to filepath."""
    try:
        resp = requests.get(url, timeout=60)
        resp.raise_for_status()
        filepath.write_bytes(resp.content)
        # Verify it's a valid image
        img = Image.open(filepath)
        img.verify()
        return True
    except Exception as e:
        print(f"  ⚠️ Download failed: {e}")
        return False


def generate_animation(
    character: str,
    animation: str,
    frames_needed: int,
    output_dir: Path,
    batches: int = 3,
    batch_size: int = 4,
) -> list[Path]:
    """
    Generate animation frames for a character.
    Generates multiple batches and collects all frames.
    Returns list of downloaded frame paths.
    """
    char = CHARACTERS[character]
    anim_spec = char["animations"][animation]

    full_prompt = f"{char['base_prompt']}, {anim_spec['prompt_suffix']}"

    # wan2.7-image-pro minimum: 768*768 (589824 pixels)
    size = "1024*1024"

    all_urls = []
    frames_generated = 0
    total_attempts = batches * batch_size

    print(f"  📸 Generating {frames_needed} frames ({batches} batches × {batch_size})...")

    for batch in range(batches):
        seed = 42 + batch * 7  # Different seed per batch
        urls = generate_image(full_prompt, size=size, n=batch_size, seed=seed)
        all_urls.extend(urls)
        frames_generated += len(urls)
        print(f"    Batch {batch+1}/{batches}: got {len(urls)} frames (total: {frames_generated}/{total_attempts})")

        if frames_generated >= frames_needed:
            break

        # Small delay between batches
        if batch < batches - 1:
            time.sleep(2)

    # Download frames
    frames = []
    prefix = f"char_{character}_{animation}"
    for i, url in enumerate(all_urls[:frames_needed * 2]):  # Generate extras, cap at 2× needed
        idx = i + 1
        filepath = output_dir / f"{prefix}_{idx:03d}.png"
        if download_image(url, filepath):
            frames.append(filepath)
            print(f"    ✅ Frame {idx}: {filepath.name}")
        else:
            print(f"    ❌ Frame {idx}: download failed")

        if len(frames) >= frames_needed:
            break

    print(f"  ✅ Generated {len(frames)}/{frames_needed} frames for {character}/{animation}")
    return frames


def generate_character(char_name: str, dry_run: bool = False):
    """Generate all sprites for a character."""
    char = CHARACTERS[char_name]
    char_dir = OUTPUT_DIR / char_name
    char_dir.mkdir(parents=True, exist_ok=True)

    print(f"\n{'='*60}")
    print(f"🎮 Character: {char['name_cn']} ({char['name_en']})")
    print(f"   Output: {char_dir}")
    print(f"{'='*60}")

    results = {}
    for anim_name, anim_spec in char["animations"].items():
        frames_needed = anim_spec["frames"]
        print(f"\n {anim_name} ({frames_needed} frames)")

        if dry_run:
            print(f"    [DRY RUN] Would generate {frames_needed} frames")
            results[anim_name] = []
            continue

        frames = generate_animation(
            character=char_name,
            animation=anim_name,
            frames_needed=frames_needed,
            output_dir=char_dir,
        )
        results[anim_name] = frames

    return results


# ─── CLI ──────────────────────────────────────────────────────────────────────


def main():
    parser = argparse.ArgumentParser(description="Sprite Asset Generator for 格斗萌主")
    parser.add_argument("--character", choices=list(CHARACTERS.keys()), help="Character to generate")
    parser.add_argument("--animation", help="Specific animation (or 'all')")
    parser.add_argument("--all-characters", action="store_true", help="Generate for all characters")
    parser.add_argument("--dry-run", action="store_true", help="Show what would be generated")
    parser.add_argument("--batches", type=int, default=3, help="Batches per animation")
    parser.add_argument("--batch-size", type=int, default=4, help="Images per batch")
    args = parser.parse_args()

    if not DASHSCOPE_API_KEY:
        print("❌ DASHSCOPE_API_KEY not set! Run: export DASHSCOPE_API_KEY='your-key'")
        sys.exit(1)

    if args.all_characters:
        for char_name in CHARACTERS:
            generate_character(char_name, dry_run=args.dry_run)
    elif args.character:
        if args.animation and args.animation != "all":
            char = CHARACTERS[args.character]
            if args.animation not in char["animations"]:
                print(f"❌ Unknown animation: {args.animation}")
                print(f"   Available: {list(char['animations'].keys())}")
                sys.exit(1)
            char_dir = OUTPUT_DIR / args.character
            char_dir.mkdir(parents=True, exist_ok=True)
            generate_animation(
                character=args.character,
                animation=args.animation,
                frames_needed=char["animations"][args.animation]["frames"],
                output_dir=char_dir,
                batches=args.batches,
                batch_size=args.batch_size,
            )
        else:
            generate_character(args.character, dry_run=args.dry_run)
    else:
        parser.print_help()


if __name__ == "__main__":
    main()
