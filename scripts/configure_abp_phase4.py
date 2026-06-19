"""
ABP Configuration - Phase 4: Fix variable type + find graph node insertion
============================================================================
Phase 3 findings:
- AnimGraphNode_StateMachine() can be created (outer = /Engine/Transient)
- No API to add it to AnimGraph
- add_member_variable needs correct type parameter
- AnimGraphLibrary exists - need to explore
"""
import unreal
import traceback

LOG = "[ABP4]"

def log(msg):
    unreal.log(f"{LOG} {msg}")

def log_error(msg):
    unreal.log_error(f"{LOG} {msg}")

ABP_PATH = "/Game/Characters/Huikong/ABP/ABP_Huikong"
abp = unreal.load_asset(ABP_PATH)
graphs = abp.get_animation_graphs()
anim_graph = graphs[0]

log(f"ABP: {abp.get_name()}, Graph: {anim_graph.get_name()}")

# ═══════════════════════════════════════════════════════════════
# STEP 1: Fix Speed variable creation
# ═══════════════════════════════════════════════════════════════
log("\n=== Step 1: Speed Variable (fix type) ===")

bel = unreal.BlueprintEditorLibrary
add_var = bel.add_member_variable

# Try different type specifications
attempts = [
    ("float", "string 'float'"),
    ("Float", "string 'Float'"),
    ("double", "string 'double'"),
    ("int", "string 'int'"),
    ("bool", "string 'bool'"),
    ("Name", "string 'Name'"),
]

for type_val, desc in attempts:
    try:
        result = add_var(abp, "Speed", type_val)
        log(f"  SUCCESS with {desc}: {result}")
        break
    except Exception as e:
        log(f"  Failed with {desc}: {type(e).__name__}: {str(e)[:100]}")

# Also try: maybe it needs EdGraphPinType
try:
    # Check what EdGraphPinType looks like
    pin = unreal.EdGraphPinType
    log(f"  EdGraphPinType attrs: {[a for a in dir(pin) if not a.startswith('_')]}")
except Exception as e:
    log(f"  EdGraphPinType: {e}")

# Try with unreal.Struct
try:
    result = add_var(abp, "Speed", unreal.EdGraphPinType)
    log(f"  SUCCESS with EdGraphPinType class: {result}")
except Exception as e:
    log(f"  EdGraphPinType class failed: {e}")

# ═══════════════════════════════════════════════════════════════
# STEP 2: Explore AnimGraphLibrary
# ═══════════════════════════════════════════════════════════════
log("\n=== Step 2: AnimGraphLibrary ===")

agl = unreal.AnimGraphLibrary
methods = [m for m in dir(agl) if not m.startswith('_')]
log(f"  Methods: {methods}")

# ═══════════════════════════════════════════════════════════════
# STEP 3: Try to insert state machine into graph
# ═══════════════════════════════════════════════════════════════
log("\n=== Step 3: Insert StateMachine into Graph ===")

# Create state machine node
sm_node = unreal.AnimGraphNode_StateMachine()
log(f"  Created: {sm_node}")
log(f"  Outer: {sm_node.get_outer()}")
log(f"  Path: {sm_node.get_path_name()}")

# Method 1: Try to rename/reparent to the graph
try:
    sm_node.rename("StateMachine_0", anim_graph)
    log(f"  After rename: outer={sm_node.get_outer()}, path={sm_node.get_path_name()}")
except Exception as e:
    log(f"  rename failed: {e}")

# Method 2: Try to call_method on graph to add node
try:
    graph_methods = [m for m in dir(anim_graph) if 'call' in m.lower() or 'method' in m.lower()]
    log(f"  Graph call methods: {graph_methods}")

    # Try call_method
    result = anim_graph.call_method("AddNode", [sm_node])
    log(f"  call_method AddNode: {result}")
except Exception as e:
    log(f"  call_method failed: {e}")

# Method 3: Try set_editor_property on graph
try:
    # Check graph's properties
    props = []
    for prop_name in ['Nodes', 'GraphNodes', 'AllNodes', 'NodeList']:
        try:
            val = anim_graph.get_editor_property(prop_name)
            props.append(f"{prop_name}={val}")
        except:
            pass
    log(f"  Graph properties found: {props}")
except Exception as e:
    log(f"  Property check failed: {e}")

# Method 4: Try creating node with graph as outer via NewObject
try:
    # Check if unreal.new_object or similar exists
    new_obj_funcs = [c for c in dir(unreal) if 'new_object' in c.lower() or 'NewObject' in c]
    log(f"  NewObject functions: {new_obj_funcs}")

    # Check if we can use construct_object or similar
    construct_funcs = [c for c in dir(unreal) if 'construct' in c.lower() and 'object' in c.lower()]
    log(f"  Construct functions: {construct_funcs}")

    # Try Object library
    for lib_name in ['ObjectLibrary', 'ObjectTools', 'EditorUtilityLibrary']:
        lib = getattr(unreal, lib_name, None)
        if lib:
            relevant = [m for m in dir(lib) if 'create' in m.lower() or 'new' in m.lower() or 'make' in m.lower()]
            if relevant:
                log(f"  {lib_name}: {relevant}")
except Exception as e:
    log(f"  NewObject exploration failed: {e}")

# ═══════════════════════════════════════════════════════════════
# STEP 4: Alternative - Animation Asset approach
# ═══════════════════════════════════════════════════════════════
log("\n=== Step 4: Alternative - Animation Asset ===")

# Check if we can create a regular Animation Blueprint without state machine
# Or use Animation Asset directly
try:
    # Check what animation-related assets we can create
    anim_factories = [c for c in dir(unreal) if 'Factory' in c and 'Anim' in c]
    log(f"  Anim factories: {anim_factories}")

    # Check for AnimComposite / AnimMontage
    for cls_name in ['AnimComposite', 'AnimMontage', 'AnimSequence']:
        cls = getattr(unreal, cls_name, None)
        if cls:
            methods = [m for m in dir(cls) if 'section' in m.lower() or 'add' in m.lower() or 'slot' in m.lower()]
            log(f"  {cls_name}: {methods}")
except Exception as e:
    log(f"  Alternative exploration failed: {e}")

# ═══════════════════════════════════════════════════════════════
# STEP 5: Check call_method on SM node
# ═══════════════════════════════════════════════════════════════
log("\n=== Step 5: SM Node methods ===")

try:
    sm_node2 = unreal.AnimGraphNode_StateMachine()
    # Explore its properties
    for prop_name in ['StateMachineName', 'Name', 'States', 'Transitions',
                       'EntryNode', 'DefaultNode', 'NodeIndex']:
        try:
            val = sm_node2.get_editor_property(prop_name)
            log(f"  {prop_name} = {repr(val)[:100]}")
        except Exception as e:
            pass

    # Try call_method with various names
    for method_name in ['CreateState', 'AddState', 'GetStates',
                         'CreateTransition', 'AddTransition',
                         'Initialize', 'Setup']:
        try:
            result = sm_node2.call_method(method_name, [])
            log(f"  call_method({method_name}): {result}")
        except Exception as e:
            pass
except Exception as e:
    log(f"  SM node exploration failed: {e}")

# ═══════════════════════════════════════════════════════════════
# STEP 6: Check AnimGraphNode_SequencePlayer for direct use
# ═══════════════════════════════════════════════════════════════
log("\n=== Step 6: SequencePlayer approach ===")

try:
    sp = unreal.AnimGraphNode_SequencePlayer()
    log(f"  SequencePlayer created: {sp}")
    sp_methods = [m for m in dir(sp) if not m.startswith('_')]
    log(f"  Methods: {sp_methods}")

    # Try to set animation
    for prop in ['Sequence', 'Animation', 'AnimSequence', 'Pose']:
        try:
            val = sp.get_editor_property(prop)
            log(f"  {prop} = {val}")
        except:
            pass

    # Try to set the animation
    try:
        idle_anim = unreal.load_asset("/Game/Characters/Huikong/Animations/AM_Huikong_Idle_01")
        sp.set_editor_property("Sequence", idle_anim)
        log(f"  Set Sequence to Idle: OK")
        log(f"  Sequence now: {sp.get_editor_property('Sequence')}")
    except Exception as e:
        log(f"  Set Sequence error: {e}")

except Exception as e:
    log(f"  SequencePlayer error: {e}")

log("\n=== Phase 4 Complete ===")

# Save
try:
    unreal.EditorAssetLibrary.save_asset(ABP_PATH)
    log("Saved")
except:
    pass
