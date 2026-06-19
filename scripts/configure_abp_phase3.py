"""
ABP Configuration - Phase 3: Actually configure the ABP
=========================================================
Using discovered APIs:
1. BlueprintEditorLibrary.add_member_variable → create Speed variable
2. AnimationGraph.get_graph_nodes_of_class → explore graph
3. AnimGraphNode_StateMachine → try to create state machine
4. AnimationStateMachineLibrary → check for state creation utilities
"""
import unreal
import traceback

LOG = "[ABP3]"

def log(msg):
    unreal.log(f"{LOG} {msg}")

def log_error(msg):
    unreal.log_error(f"{LOG} {msg}")

# ─── Load ABP ────────────────────────────────────────────────
ABP_PATH = "/Game/Characters/Huikong/ABP/ABP_Huikong"
abp = unreal.load_asset(ABP_PATH)
if not abp:
    log_error("Cannot load ABP")
    exit(1)
log(f"ABP loaded: {abp.get_name()}")

# Get AnimGraph
graphs = abp.get_animation_graphs()
anim_graph = graphs[0] if graphs else None
if not anim_graph:
    log_error("No AnimGraph found")
    exit(1)
log(f"AnimGraph: {anim_graph.get_name()}")

# ═══════════════════════════════════════════════════════════════
# STEP 1: Create Speed variable
# ═══════════════════════════════════════════════════════════════
log("\n=== Step 1: Create Speed Variable ===")

try:
    # BlueprintEditorLibrary.add_member_variable
    # Signature: add_member_variable(blueprint, variable_name, variable_type, ...)
    log("Exploring BlueprintEditorLibrary.add_member_variable...")

    bel = unreal.BlueprintEditorLibrary
    add_var_method = getattr(bel, 'add_member_variable', None)
    if add_var_method:
        # Check method signature
        sig = [a for a in dir(add_var_method) if not a.startswith('_')]
        log(f"  add_member_variable attrs: {sig}")

        # Try to call it
        # Typical signature: add_member_variable(Blueprint, Name, Type, [DefaultValue])
        try:
            # Try with unreal.Name for variable name
            result = bel.add_member_variable(
                abp,
                "Speed",
                unreal.FloatProperty  # or "float"
            )
            log(f"  SUCCESS: Speed variable created! Result: {result}")
        except TypeError as te:
            log(f"  TypeError (trying alternative signature): {te}")
            try:
                # Try with string type
                result = bel.add_member_variable(abp, "Speed", "float")
                log(f"  SUCCESS (string type): {result}")
            except Exception as e2:
                log(f"  Also failed: {e2}")
                try:
                    # Try with unreal.EdGraphPinType
                    pin_type = unreal.EdGraphPinType(
                        unreal.EdGraphPinType.PIN_TYPE_FLOAT,
                        None, None, False, False
                    )
                    result = bel.add_member_variable(abp, "Speed", pin_type)
                    log(f"  SUCCESS (pin type): {result}")
                except Exception as e3:
                    log_error(f"  All attempts failed: {e3}")
    else:
        log_error("  add_member_variable not found!")

except Exception as e:
    log_error(f"Variable creation error: {e}")
    log_error(traceback.format_exc())

# ═══════════════════════════════════════════════════════════════
# STEP 2: Explore AnimationStateMachineLibrary
# ═══════════════════════════════════════════════════════════════
log("\n=== Step 2: AnimationStateMachineLibrary ===")

try:
    asml = unreal.AnimationStateMachineLibrary
    methods = [m for m in dir(asml) if not m.startswith('_')]
    log(f"  Methods: {methods}")

    # Look for state creation methods
    state_methods = [m for m in methods if 'state' in m.lower() or 'create' in m.lower() or 'add' in m.lower() or 'machine' in m.lower()]
    log(f"  State-related methods: {state_methods}")

except Exception as e:
    log_error(f"ASML error: {e}")

# ═══════════════════════════════════════════════════════════════
# STEP 3: Explore AnimGraphNode_StateMachine
# ═══════════════════════════════════════════════════════════════
log("\n=== Step 3: AnimGraphNode_StateMachine ===")

try:
    sm_cls = unreal.AnimGraphNode_StateMachine
    methods = [m for m in dir(sm_cls) if not m.startswith('_')]
    log(f"  Methods ({len(methods)}): {methods}")

    # Try to create an instance
    try:
        sm_node = unreal.AnimGraphNode_StateMachine()
        log(f"  Created instance: {sm_node}")
        log(f"  Instance type: {type(sm_node)}")
    except Exception as e:
        log(f"  Cannot create instance directly: {e}")

except Exception as e:
    log_error(f"StateMachine node error: {e}")

# ═══════════════════════════════════════════════════════════════
# STEP 4: Check AnimationGraph for node creation
# ═══════════════════════════════════════════════════════════════
log("\n=== Step 4: AnimationGraph node operations ===")

try:
    graph_methods = [m for m in dir(anim_graph) if not m.startswith('_')]
    log(f"  AnimationGraph methods: {graph_methods}")

    # Check get_graph_nodes_of_class
    try:
        existing_nodes = anim_graph.get_graph_nodes_of_class(unreal.AnimGraphNode_StateMachine)
        log(f"  Existing StateMachine nodes: {len(existing_nodes)}")
    except Exception as e:
        log(f"  get_graph_nodes_of_class(StateMachine) error: {e}")

    # Try to find a method to add nodes
    add_methods = [m for m in graph_methods if 'add' in m.lower() or 'create' in m.lower() or 'insert' in m.lower() or 'new' in m.lower()]
    log(f"  Add/Create methods: {add_methods}")

    # Check if AnimationGraph has schema
    try:
        schema = anim_graph.get_editor_property("Schema")
        log(f"  Schema: {schema}")
        schema_methods = [m for m in dir(schema) if not m.startswith('_')]
        create_methods = [m for m in schema_methods if 'create' in m.lower() or 'add' in m.lower()]
        log(f"  Schema create methods: {create_methods}")
    except Exception as e:
        log(f"  Schema error: {e}")

except Exception as e:
    log_error(f"AnimationGraph error: {e}")
    log_error(traceback.format_exc())

# ═══════════════════════════════════════════════════════════════
# STEP 5: Try EditorActorSubsystem or other subsystems
# ═══════════════════════════════════════════════════════════════
log("\n=== Step 5: Alternative approaches ===")

try:
    # Check if there's a way to execute console commands
    # which might allow us to manipulate the graph
    for sub_name in ['EditorActorSubsystem', 'UnrealEditorSubsystem', 'AssetEditorSubsystem']:
        sub = getattr(unreal, sub_name, None)
        if sub:
            sub_methods = [m for m in dir(sub) if not m.startswith('_')]
            relevant = [m for m in sub_methods if 'command' in m.lower() or 'exec' in m.lower() or 'console' in m.lower() or 'python' in m.lower()]
            if relevant:
                log(f"  {sub_name} relevant: {relevant}")
except Exception as e:
    log_error(f"Subsystem error: {e}")

# ═══════════════════════════════════════════════════════════════
# STEP 6: Check AnimStateNodeBase
# ═══════════════════════════════════════════════════════════════
log("\n=== Step 6: AnimStateNodeBase ===")

try:
    state_node_cls = unreal.AnimStateNodeBase
    methods = [m for m in dir(state_node_cls) if not m.startswith('_')]
    log(f"  Methods: {methods}")

    # Try to create
    try:
        state_node = unreal.AnimStateNodeBase()
        log(f"  Created: {state_node}")
    except Exception as e:
        log(f"  Cannot create: {e}")
except Exception as e:
    log_error(f"AnimStateNodeBase error: {e}")

# ═══════════════════════════════════════════════════════════════
# STEP 7: Check AnimStateTransitionNode
# ═══════════════════════════════════════════════════════════════
log("\n=== Step 7: AnimStateTransitionNode ===")

try:
    trans_cls = unreal.AnimStateTransitionNode
    methods = [m for m in dir(trans_cls) if not m.startswith('_')]
    log(f"  Methods: {methods}")
except Exception as e:
    log_error(f"TransitionNode error: {e}")

# ═══════════════════════════════════════════════════════════════
# STEP 8: Try AnimationStateGraph
# ═══════════════════════════════════════════════════════════════
log("\n=== Step 8: AnimationStateGraph ===")

try:
    asg_cls = unreal.AnimationStateGraph
    methods = [m for m in dir(asg_cls) if not m.startswith('_')]
    log(f"  Methods: {methods}")
except Exception as e:
    log_error(f"AnimationStateGraph error: {e}")

# ═══════════════════════════════════════════════════════════════
# STEP 9: Try AnimationStateEntry
# ═══════════════════════════════════════════════════════════════
log("\n=== Step 9: AnimationStateEntry ===")

try:
    ase_cls = unreal.AnimationStateEntry
    methods = [m for m in dir(ase_cls) if not m.startswith('_')]
    log(f"  Methods: {methods}")
except Exception as e:
    log_error(f"AnimationStateEntry error: {e}")

# ═══════════════════════════════════════════════════════════════
# STEP 10: Check GraphEditor library functions
# ═══════════════════════════════════════════════════════════════
log("\n=== Step 10: Graph editor functions ===")

try:
    # Look for graph-related functions in unreal module
    graph_funcs = [c for c in dir(unreal) if 'GraphEditor' in c and not c.startswith('_')]
    log(f"  GraphEditor classes: {graph_funcs}")

    # Look for node creation utilities
    node_utils = [c for c in dir(unreal) if 'NodeFactory' in c or 'GraphFactory' in c or 'NodeCreation' in c]
    log(f"  Node factory classes: {node_utils}")

    # Check for AnimationGraphEditor
    age_classes = [c for c in dir(unreal) if 'AnimationGraphEditor' in c or 'AnimGraphEditor' in c]
    log(f"  AnimGraphEditor classes: {age_classes}")

    # Check for AnimBlueprintEditor
    abe_classes = [c for c in dir(unreal) if 'AnimBlueprintEditor' in c]
    log(f"  AnimBlueprintEditor classes: {abe_classes}")

    # Check for AnimGraphEditorUtils
    agu_classes = [c for c in dir(unreal) if 'AnimGraph' in c and ('Util' in c or 'Lib' in c or 'Helper' in c)]
    log(f"  AnimGraph utility classes: {agu_classes}")

except Exception as e:
    log_error(f"Graph editor error: {e}")

log("\n=== Phase 3 Complete ===")

# Save ABP
try:
    unreal.EditorAssetLibrary.save_asset(ABP_PATH)
    log("ABP saved")
except Exception as e:
    log_error(f"Save error: {e}")
