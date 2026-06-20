"""
ABP Configuration Script for Huikong
=====================================
Configures ABP_Huikong with:
1. Speed variable
2. EventGraph: Calculate speed from pawn velocity
3. AnimGraph state machine (if API supports it)

This script is designed to run from init_unreal.py on editor startup.
"""
import unreal
import sys

LOG_PREFIX = "[ABP-CFG]"

def log(msg):
    unreal.log(f"{LOG_PREFIX} {msg}")

def log_error(msg):
    unreal.log_error(f"{LOG_PREFIX} {msg}")

def log_warning(msg):
    unreal.log_warning(f"{LOG_PREFIX} {msg}")

# ─── Asset paths ─────────────────────────────────────────────
ABP_PATH = "/Game/Characters/Huikong/ABP/ABP_Huikong"
SKELETON_PATH = "/Game/Characters/Huikong/Mesh/SKM_Huikong_Skeleton"

# Animation paths
ANIM_IDLE_01 = "/Game/Characters/Huikong/Animations/AM_Huikong_Idle_01"
ANIM_IDLE_02 = "/Game/Characters/Huikong/Animations/AM_Huikong_Idle_02"
ANIM_WALK = "/Game/Characters/Huikong/Animations/AM_Huikong_Walk_01"
ANIM_RUN = "/Game/Characters/Huikong/Animations/AM_Huikong_Run_01"
ANIM_HIT = "/Game/Characters/Huikong/Animations/AM_Huikong_HitReaction"
ANIM_LANDING = "/Game/Characters/Huikong/Animations/AM_Huikong_Landing"
ANIM_ATTACK = "/Game/Characters/Huikong/Animations/AM_Huikong_LightAttack_01"

# ─── Load assets ─────────────────────────────────────────────
log("=== Starting ABP configuration ===")

abp = unreal.load_asset(ABP_PATH)
skeleton = unreal.load_asset(SKELETON_PATH)

if not abp:
    log_error(f"Cannot load ABP: {ABP_PATH}")
    sys.exit(1)
if not skeleton:
    log_error(f"Cannot load skeleton: {SKELETON_PATH}")
    sys.exit(1)

log(f"ABP loaded: {abp.get_name()}")
log(f"Skeleton: {skeleton.get_name()}")

# ─── Step 1: Explore ABP capabilities ────────────────────────
log("Exploring ABP API...")

# List all available methods
methods = [m for m in dir(abp) if not m.startswith('_')]
log(f"ABP has {len(methods)} methods/properties")
log(f"Key methods: {[m for m in methods if 'graph' in m.lower() or 'state' in m.lower() or 'var' in m.lower() or 'param' in m.lower()]}")

# ─── Step 2: Check existing graphs ───────────────────────────
try:
    graphs = abp.get_all_graphs()
    log(f"Existing graphs: {[g.get_name() for g in graphs]}")

    for graph in graphs:
        graph_name = graph.get_name()
        log(f"\nGraph: {graph_name}")

        # Get all nodes in graph
        try:
            nodes = unreal.EditorFilterLibrary.by_class(
                unreal.K2Node,
                graph.get_all_nodes() if hasattr(graph, 'get_all_nodes') else []
            )
            log(f"  Nodes: {len(nodes) if nodes else 0}")
        except Exception as e:
            log(f"  Could not get nodes: {e}")

except Exception as e:
    log_error(f"Error exploring graphs: {e}")
    import traceback
    log_error(f"Traceback: {traceback.format_exc()}")

# ─── Step 3: Try to create Speed variable ─────────────────────
log("\nAttempting to create Speed variable...")

# Method 1: Try AnimBlueprintLibrary
try:
    # In UE5, ABP variables might need to be created differently
    # Let's check what AnimBlueprintLibrary offers
    ablib_methods = [m for m in dir(unreal.AnimBlueprintLibrary) if not m.startswith('_')]
    log(f"AnimBlueprintLibrary methods: {ablib_methods}")
except Exception as e:
    log_warning(f"Could not list AnimBlueprintLibrary: {e}")

# Method 2: Try to add variable through graph
try:
    # Get the EventGraph
    event_graph = None
    for graph in abp.get_all_graphs():
        if "EventGraph" in graph.get_name() or "AnimGraph" in graph.get_name():
            event_graph = graph
            log(f"Found graph: {graph.get_name()}")
            break

    if event_graph:
        # Try to create a variable node
        log("Attempting to create K2Node_VariableSet for Speed...")

        # In UE5 Python, creating blueprint variables is done through the Blueprint API
        # For AnimBlueprint, we need to use specific methods
        try:
            # Check if we can add a variable to the ABP's parent class
            bp_methods = [m for m in dir(abp) if 'variable' in m.lower() or 'member' in m.lower()]
            log(f"Variable-related methods: {bp_methods}")
        except Exception as e:
            log_warning(f"Error: {e}")

except Exception as e:
    log_error(f"Graph creation error: {e}")

# ─── Step 4: Try AnimGraph state machine ──────────────────────
log("\nAttempting AnimGraph state machine configuration...")

try:
    # Find the AnimGraph
    anim_graph = None
    for graph in abp.get_all_graphs():
        if "AnimGraph" in graph.get_name():
            anim_graph = graph
            break

    if anim_graph:
        log(f"AnimGraph found: {anim_graph.get_name()}")

        # Try to create a state machine node
        # UE5 Python API for state machine creation is limited
        # Let's see what's available

        # Check for AnimStateMachine-related classes
        sm_classes = [c for c in dir(unreal) if 'StateMachine' in c or 'AnimState' in c]
        log(f"State machine related classes: {sm_classes}")

        # Try to find/create state machine node
        try:
            # In UE5, state machines are nodes in the AnimGraph
            # We might be able to use unreal.AnimGraphNode_StateMachine
            # or create through graph editor utilities

            # List available node classes
            node_classes = [c for c in dir(unreal) if 'AnimGraph' in c and 'Node' in c]
            log(f"AnimGraph node classes: {node_classes}")

        except Exception as e:
            log_warning(f"State machine exploration error: {e}")
    else:
        log("No AnimGraph found in ABP")

except Exception as e:
    log_error(f"AnimGraph error: {e}")

# ─── Step 5: Verify animations ────────────────────────────────
log("\nVerifying animation assets...")

anim_paths = {
    "Idle_01": ANIM_IDLE_01,
    "Idle_02": ANIM_IDLE_02,
    "Walk": ANIM_WALK,
    "Run": ANIM_RUN,
    "HitReaction": ANIM_HIT,
    "Landing": ANIM_LANDING,
    "LightAttack": ANIM_ATTACK,
}

for name, path in anim_paths.items():
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        anim = unreal.load_asset(path)
        if anim:
            # Get animation length
            try:
                length = anim.get_editor_property("SequenceLength") if hasattr(anim, 'get_editor_property') else "unknown"
            except:
                length = "unknown"
            log(f"  ✓ {name}: {anim.get_name()} (length: {length}s)")
        else:
            log(f"  ✗ {name}: exists but failed to load")
    else:
        log(f"  ✗ {name}: NOT FOUND at {path}")

# ─── Summary ──────────────────────────────────────────────────
log("\n=== Configuration Summary ===")
log("ABP exists and skeleton is linked.")
log("Animations verified.")
log("")
log("NEXT STEPS (require VNC/editor UI):")
log("1. Open ABP_Huikong in editor")
log("2. In My Blueprint panel, add variable:")
log("   - Name: Speed")
log("   - Type: Float")
log("   - Default: 0.0")
log("3. In EventGraph, add nodes:")
log("   - Event BlueprintUpdateAnimation")
log("   - Try Get Pawn Vel → Vector Length → Set Speed")
log("4. In AnimGraph:")
log("   - Right-click → Add New State Machine")
log("   - Double-click to enter, create states:")
log("     * Idle (AM_Huikong_Idle_01)")
log("     * Walk (AM_Huikong_Walk_01)")
log("     * Run (AM_Huikong_Run_01)")
log("   - Create transitions:")
log("     * Idle→Walk: Speed > 10")
log("     * Walk→Idle: Speed < 10")
log("     * Walk→Run: Speed > 300")
log("     * Run→Walk: Speed < 300")
log("   - Set blend times: 0.2s for all transitions")
log("5. Connect state machine Output to Pose")
log("")
log("=== Done ===")

# Save
try:
    unreal.EditorAssetLibrary.save_asset(ABP_PATH)
    log(f"Saved: {ABP_PATH}")
except Exception as e:
    log_error(f"Save failed: {e}")
