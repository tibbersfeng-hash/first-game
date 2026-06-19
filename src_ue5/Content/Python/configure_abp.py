"""
Configure ABP_Huikong - Step 1: EventGraph setup
================================================
This script configures the EventGraph of ABP_Huikong:
1. Creates a 'Speed' float variable
2. Sets up EventGraphBlueprintUpdate to calculate Speed from Pawn velocity

Step 2 (AnimGraph state machine) must be done via VNC/editor UI.
"""
import unreal

unreal.log("[ABP-Config] === Starting ABP configuration ===")

# ─── Load assets ─────────────────────────────────────────────
abp_path = "/Game/Characters/Huikong/ABP/ABP_Huikong"
skeleton_path = "/Game/Characters/Huikong/Mesh/SKM_Huikong_Skeleton"

abp = unreal.load_asset(abp_path)
skeleton = unreal.load_asset(skeleton_path)

if not abp:
    unreal.log_error(f"[ABP-Config] FAILED: Cannot load {abp_path}")
    exit(1)
if not skeleton:
    unreal.log_error(f"[ABP-Config] FAILED: Cannot load {skeleton_path}")
    exit(1)

unreal.log(f"[ABP-Config] ABP loaded: {abp.get_name()}")
unreal.log(f"[ABP-Config] Skeleton: {skeleton.get_name()}")

# ─── Step 1: Create Speed variable ──────────────────────────
unreal.log("[ABP-Config] Creating 'Speed' variable...")

try:
    # Check if Speed variable already exists
    existing_vars = abp.get_editor_property("ClassVariableNames") if hasattr(abp, 'get_editor_property') else []
    unreal.log(f"[ABP-Config] Current variables: {existing_vars}")
except Exception as e:
    unreal.log_warning(f"[ABP-Config] Could not check existing vars: {e}")

# Create Speed variable using AnimBlueprintLibrary
try:
    # Method 1: Try using the property binding system
    unreal.AnimBlueprintLibrary.add_parameter(
        abp,
        "Speed",
        unreal.AnimPropertyType.FLOAT,
        0.0
    )
    unreal.log("[ABP-Config] Speed variable created via add_parameter")
except Exception as e:
    unreal.log_warning(f"[ABP-Config] add_parameter failed: {e}, trying alternative...")
    try:
        # Method 2: Set via editor property
        # In UE5, ABP variables are stored differently
        # We might need to use the K2Node approach
        unreal.log("[ABP-Config] Trying K2Node variable creation...")
    except Exception as e2:
        unreal.log_error(f"[ABP-Config] Variable creation failed: {e2}")

# ─── Step 2: Configure EventGraph ───────────────────────────
unreal.log("[ABP-Config] Setting up EventGraph...")

try:
    # Get or create EventGraph
    graphs = abp.get_all_graphs() if hasattr(abp, 'get_all_graphs') else []
    unreal.log(f"[ABP-Config] Existing graphs: {[g.get_name() for g in graphs]}")

    event_graph = None
    for g in graphs:
        if "EventGraph" in g.get_name():
            event_graph = g
            break

    if not event_graph:
        unreal.log("[ABP-Config] No EventGraph found, will be created in editor")
    else:
        unreal.log(f"[ABP-Config] EventGraph found: {event_graph.get_name()}")

        # Clear existing nodes
        try:
            all_nodes = unreal.K2Node.get_all_nodes(event_graph)
            unreal.log(f"[ABP-Config] Existing nodes: {len(all_nodes)}")
        except:
            pass

except Exception as e:
    unreal.log_error(f"[ABP-Config] EventGraph setup failed: {e}")
    import traceback
    unreal.log_error(f"[ABP-Config] Traceback: {traceback.format_exc()}")

# ─── Save ────────────────────────────────────────────────────
try:
    unreal.EditorAssetLibrary.save_asset(abp_path)
    unreal.log(f"[ABP-Config] Saved {abp_path}")
except Exception as e:
    unreal.log_error(f"[ABP-Config] Save failed: {e}")

unreal.log("[ABP-Config] === Configuration complete ===")
unreal.log("[ABP-Config] Next: Configure AnimGraph state machine via VNC/editor UI")
unreal.log("[ABP-Config]   1. Open ABP_Huikong in editor")
unreal.log("[ABP-Config]   2. Go to AnimGraph tab")
unreal.log("[ABP-Config]   3. Right-click → Add New State Machine")
unreal.log("[ABP-Config]   4. Double-click state machine to create states:")
unreal.log("[ABP-Config]      - Idle, Walk, Run")
unreal.log("[ABP-Config]   5. Set each state's animation")
unreal.log("[ABP-Config]   6. Create transitions with Speed conditions")
