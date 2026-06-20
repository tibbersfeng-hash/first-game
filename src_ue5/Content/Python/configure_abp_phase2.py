"""
ABP Configuration - Phase 2: Deep API Exploration
===================================================
Using correct method names discovered in Phase 1.
"""
import unreal
import traceback

LOG = "[ABP2]"

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

# ─── 1. List ALL ABP methods/properties ──────────────────────
log("=== ABP Full API ===")
all_attrs = sorted([a for a in dir(abp) if not a.startswith('_')])
for attr in all_attrs:
    try:
        val = getattr(abp, attr)
        if callable(val):
            log(f"  METHOD: {attr}")
        else:
            log(f"  PROP: {attr} = {repr(val)[:100]}")
    except Exception as e:
        log(f"  ATTR: {attr} (error: {e})")

# ─── 2. Explore Animation Graphs ─────────────────────────────
log("\n=== Animation Graphs ===")
try:
    graphs = abp.get_animation_graphs()
    log(f"Graphs: {graphs}")
    if graphs:
        for graph in graphs:
            log(f"\n  Graph: {graph}")
            log(f"  Type: {type(graph)}")
            graph_attrs = [a for a in dir(graph) if not a.startswith('_')]
            log(f"  Methods: {graph_attrs[:30]}")
except Exception as e:
    log_error(f"get_animation_graphs error: {e}")
    log_error(traceback.format_exc())

# ─── 3. Try Blueprint graph access via parent class ──────────
log("\n=== Blueprint Graph Access ===")
try:
    # AnimBlueprint inherits from Blueprint
    # Try Blueprint-level methods
    bp_methods = [a for a in dir(abp) if 'graph' in a.lower()]
    log(f"Graph-related attrs: {bp_methods}")

    # Try to get graphs via different methods
    for method_name in ['get_all_graphs', 'get_graphs', 'graphs',
                         'get_animation_graphs', 'get_graph']:
        try:
            method = getattr(abp, method_name, None)
            if method:
                result = method() if callable(method) else method
                log(f"  {method_name}() = {result}")
        except Exception as e:
            log(f"  {method_name}() failed: {e}")
except Exception as e:
    log_error(f"Blueprint graph access error: {e}")

# ─── 4. Explore variable creation ────────────────────────────
log("\n=== Variable Creation ===")
try:
    # Check for variable-related methods
    var_methods = [a for a in dir(abp) if 'variable' in a.lower() or 'member' in a.lower()]
    log(f"Variable methods: {var_methods}")

    # Try to find Blueprint-level variable methods
    for method_name in ['add_variable', 'create_variable', 'new_variable',
                         'find_variable', 'get_variable', 'set_variable']:
        method = getattr(abp, method_name, None)
        if method:
            log(f"  Found: {method_name}")

    # Try unreal.EditorBlueprintLibrary or similar
    for lib_name in ['EditorBlueprintLibrary', 'BlueprintEditorLibrary',
                      'KismetSystemLibrary', 'EditorActorSubsystem']:
        lib = getattr(unreal, lib_name, None)
        if lib:
            lib_methods = [a for a in dir(lib) if 'variable' in a.lower()]
            if lib_methods:
                log(f"  {lib_name} variable methods: {lib_methods}")
except Exception as e:
    log_error(f"Variable creation error: {e}")

# ─── 5. Explore graph node creation ──────────────────────────
log("\n=== Graph Node Classes ===")
try:
    # Find AnimGraph-related node classes
    anim_node_classes = [c for c in dir(unreal) if 'AnimGraph' in c and not c.startswith('_')]
    log(f"AnimGraph classes: {anim_node_classes}")

    state_classes = [c for c in dir(unreal) if 'State' in c and 'Anim' in c and not c.startswith('_')]
    log(f"Anim State classes: {state_classes}")

    # Find graph editor utilities
    graph_utils = [c for c in dir(unreal) if 'Graph' in c and 'Editor' in c and not c.startswith('_')]
    log(f"Graph editor utils: {graph_utils}")

    # Check for node creation functions
    node_funcs = [c for c in dir(unreal) if 'create_node' in c.lower() or 'add_node' in c.lower()]
    log(f"Node creation functions: {node_funcs}")

except Exception as e:
    log_error(f"Node class exploration error: {e}")

# ─── 6. Try K2Node approach ──────────────────────────────────
log("\n=== K2Node Exploration ===")
try:
    k2_classes = [c for c in dir(unreal) if c.startswith('K2Node') and ('Anim' in c or 'State' in c or 'Variable' in c)]
    log(f"K2Node relevant classes: {k2_classes}")

    # Try to find K2Node_AnimationState or similar
    for cls_name in ['K2Node_AnimState', 'K2Node_AnimationState',
                      'K2Node_StateMachine', 'K2Node_AnimStateMachine',
                      'K2Node_VariableSet', 'K2Node_VariableGet']:
        cls = getattr(unreal, cls_name, None)
        if cls:
            log(f"  Found: {cls_name}")
            cls_methods = [a for a in dir(cls) if not a.startswith('_')]
            log(f"    Methods: {cls_methods[:20]}")
except Exception as e:
    log_error(f"K2Node error: {e}")

# ─── 7. Try subsystem approach ───────────────────────────────
log("\n=== Subsystem Approach ===")
try:
    # Check for editor subsystems that might help
    subsystems = [c for c in dir(unreal) if 'Subsystem' in c and 'Editor' in c]
    log(f"Editor subsystems: {subsystems[:20]}")

    # Try ContentBrowser / Asset subsystem
    for sub_name in ['ContentBrowserSubsystem', 'EditorUtilitySubsystem',
                      'UnrealEdMisc', 'EditorLevelLibrary', 'EditorAssetLibrary']:
        sub = getattr(unreal, sub_name, None)
        if sub:
            relevant = [a for a in dir(sub) if 'python' in a.lower() or 'exec' in a.lower() or 'script' in a.lower()]
            if relevant:
                log(f"  {sub_name}: {relevant}")
except Exception as e:
    log_error(f"Subsystem error: {e}")

# ─── 8. Check for execute_python_command ─────────────────────
log("\n=== Execute Python ===")
try:
    exec_methods = [a for a in dir(unreal) if 'exec' in a.lower() and 'python' in a.lower()]
    log(f"Python exec methods: {exec_methods}")

    # Check all unreal module top-level functions
    top_funcs = [a for a in dir(unreal) if callable(getattr(unreal, a, None)) and not a.startswith('_')]
    log(f"Top-level functions ({len(top_funcs)}): {top_funcs[:50]}")
except Exception as e:
    log_error(f"Execute error: {e}")

log("\n=== Phase 2 Complete ===")
