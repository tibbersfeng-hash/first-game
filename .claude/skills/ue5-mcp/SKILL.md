---
name: ue5-mcp
description: "通过 MCP 协议远程控制 GPU 服务器上的 UE5.7 编辑器 — 生成 Actor、编辑蓝图、操作关卡、运行 PIE 等"
argument-hint: "spawn | pie | screenshot | inspect | blueprint | level | help"
user-invocable: true
allowed-tools: Read, Glob, Grep, Write, Edit, Bash, WebSearch, WebFetch
model: sonnet
---

# UE5 MCP 远程控制

> 架构: 本地 Claude → HTTP → GPU 服务器 (172.25.0.86:3000) → McpAutomationBridge → UE5.7 Editor
> 协议: MCP Streamable HTTP (JSON-RPC 2.0 over SSE)

## 连接信息

| 项目 | 值 |
|------|-----|
| 地址 | `http://172.25.0.86:3000/mcp` |
| Capability Token | `c74a40cde871789c0b27984019fc58e4` |
| Header | `X-MCP-Capability-Token` |
| Session | `Mcp-Session-Id`（initialize 返回，后续请求携带） |
| 超时 | 初始化/通知 10s，工具调用 15-30s |

## 调用流程

每次会话必须按顺序执行：

1. **POST `/mcp`** → `initialize` → 获得 `Mcp-Session-Id`
2. **POST `/mcp`** → `notifications/initialized` → 确认会话
3. **POST `/mcp`** → `tools/call` → 执行工具（SSE 流式响应）

所有请求必须带 `X-MCP-Capability-Token` header。

## 关键参数格式

```json
{
  "location": {"x": 100, "y": 200, "z": 300},
  "rotation": {"pitch": 0, "yaw": 90, "roll": 0},
  "scale": {"x": 1, "y": 1, "z": 1}
}
```

⚠️ **不是数组！** 必须是 `{x, y, z}` 对象。

## 23 个工具速查

###  核心工具

#### control_actor — Actor 生命周期
```
action: spawn | delete | duplicate | apply_force | set_transform | teleport_actor |
        set_actor_location | set_actor_rotation | set_actor_scale | set_actor_transform |
        get_transform | set_visibility | add_component | remove_component |
        set_material | get_components | get_actor_bounds |
        add_tag | remove_tag | find_by_tag | find_by_name | find_by_class |
        list | attach | detach | set_actor_collision | call_actor_function |
        create_snapshot | set_blueprint_variables
```
关键参数: `classPath`(类路径), `actorName`(标签), `location`, `scale`, `meshPath`(网格体), `blueprintPath`(蓝图)

示例 — 生成 StaticMeshActor:
```json
{"action": "spawn", "classPath": "StaticMeshActor", "actorName": "MyCube",
 "location": {"x": 500, "y": 0, "z": 200}, "scale": {"x": 3, "y": 3, "z": 3}}
```

#### control_editor — 编辑器控制
```
action: play | stop | stop_pie | pause | resume | eject | possess |
        set_camera | set_camera_position | set_viewport_camera | set_camera_fov |
        set_view_mode | set_viewport_resolution | console_command | execute_command |
        screenshot | take_screenshot | step_frame | start_recording | stop_recording |
        create_bookmark | jump_to_bookmark | open_asset | open_level |
        focus_actor | show_stats | hide_stats | set_editor_mode | set_immersive_mode |
        set_game_view | undo | redo | save_all |
        set_game_speed | set_fixed_delta_time | set_view_target |
        simulate_input | set_preferences | set_viewport_realtime
```
关键参数: `command`(控制台命令), `width`/`height`(截图分辨率), `mode`(editor_viewport|game_viewport|full_editor_window), `speed`(游戏速度), `fov`

示例 — PIE + 截图:
```json
{"action": "play"}
{"action": "screenshot", "width": 1920, "height": 1080, "mode": "editor_viewport"}
{"action": "console_command", "command": "stat fps"}
```

#### inspect — 对象内省
```
action: inspect_object | inspect_class | inspect_cdo | get_property | set_property |
        get_components | list_objects | find_by_class | find_by_tag |
        runtime_report | pie_report | get_scene_stats | get_performance_stats |
        get_memory_stats | create_snapshot | restore_snapshot | get_bounding_box |
        get_project_settings | get_world_settings | get_viewport_info |
        get_selected_actors | get_editor_settings
```
关键参数: `objectPath`, `propertyName`, `value`, `actorName`, `blueprintPath`, `componentName`

#### manage_asset — 资产管理
```
action: list | import | duplicate | rename | move | delete | create_folder |
        search_assets | get_dependencies | fixup_redirectors |
        create_material | create_material_instance | create_render_target |
        add_material_node | connect_material_pins | set_blend_mode | set_shading_model |
        set_scalar_parameter_value | set_vector_parameter_value |
        create_noise_texture | create_gradient_texture | resize_texture |
        generate_lods | nanite_rebuild_mesh | convert_to_nanite |
        source_control_checkout | source_control_submit | bulk_rename | bulk_delete
```
关键参数: `assetPath`, `sourcePath`, `destinationPath`, `newName`, `parentMaterial`, `blendMode`, `shadingModel`

#### manage_blueprint — 蓝图编辑
```
action: create | compile | add_component | set_default | modify_scs | get_scs |
        add_scs_component | remove_scs_component | set_scs_transform | set_scs_property |
        add_variable | remove_variable | rename_variable |
        add_function | remove_function | add_event | remove_event |
        add_construction_script | set_variable_metadata |
        create_node | add_node | delete_node | connect_pins | break_pin_links |
        set_node_property | create_reroute_node |
        create_widget_blueprint | add_canvas_panel | add_horizontal_box |
        add_vertical_box | add_overlay | add_button | add_text_block |
        add_image | add_scroll_box | add_list_view |
        set_anchor | set_alignment | set_position | set_size | set_visibility |
        create_property_binding | bind_text | bind_visibility | bind_on_clicked |
        create_widget_animation | add_animation_track | add_animation_keyframe
```
关键参数: `blueprintPath`, `blueprintType`(父类), `componentType`, `componentName`, `variableName`, `variableType`, `nodeType`, `fromNodeId`, `toNodeId`

#### manage_level — 关卡管理
```
action: load | save | save_as | stream | unload | create_level |
        create_light | build_lighting | set_metadata | export_level | import_level |
        list_levels | get_summary | delete | add_sublevel | rename_level |
        duplicate_level | get_current_level | validate_level
```
关键参数: `levelPath`, `lightType`(Directional|Point|Spot|Rect), `intensity`, `color`, `location`, `rotation`

#### manage_tools — 工具管理
```
action: list_tools | list_categories | enable_tools | disable_tools |
        enable_category | disable_category | get_status | reset
```
参数: `tools`(工具名数组), `category`(core|world|gameplay|utility|all)

#### system_control — 系统控制
```
action: profile | show_fps | set_quality | screenshot | set_resolution |
        set_fullscreen | console_command | execute_command | run_ubt | run_tests |
        set_cvar | get_project_settings | validate_assets | set_project_setting |
        execute_python | start_profiling | stop_profiling | run_benchmark |
        show_stats | generate_memory_report | set_scalability | set_resolution_scale |
        set_vsync | set_frame_rate_limit | configure_nanite | configure_world_partition |
        start_session | start_unreal_insights | capture_insights_trace | analyze_trace |
        spawn_category | play_sound | create_widget | show_widget
```
关键参数: `command`, `code`(Python 代码), `file`(.py 路径), `type`(CPU|GPU|Memory), `maxFPS`, `scale`

---

###  场景构建

#### build_environment — 环境搭建
```
action: create_landscape | sculpt | add_foliage | paint_foliage |
        create_procedural_terrain | create_sky_sphere | set_time_of_day |
        create_fog_volume | import_heightmap | export_heightmap |
        create_water_body_ocean | create_water_body_lake | create_water_body_river |
        configure_water_waves | configure_water_material |
        spawn_light | create_light | spawn_sky_light | create_sky_light |
        setup_global_illumination | configure_shadows | build_lighting |
        create_spline_actor | add_spline_point | set_spline_point_position |
        create_spline_mesh_component | scatter_meshes_along_spline |
        create_road_spline | create_river_spline | create_fence_spline |
        create_spline | set_spline_type | get_splines_info |
        configure_ray_traced_shadows | configure_lumen_reflection_settings |
        create_post_process_volume | set_pp_white_balance | set_pp_color_grading |
        configure_bloom | configure_dof | configure_motion_blur |
        create_scene_capture_2d | create_scene_capture_cube | capture_scene |
        configure_exposure | configure_ssao | configure_vignette |
        create_weather_system | configure_rain_particles | configure_snow_particles |
        configure_wind | configure_lightning | create_time_of_day_system
```

#### manage_level_structure — 关卡结构
```
action: create_level | create_sublevel | configure_level_streaming |
        enable_world_partition | configure_grid_size | create_data_layer |
        assign_actor_to_data_layer | configure_hlod_layer |
        create_trigger_volume | create_trigger_box | create_trigger_sphere |
        create_blocking_volume | create_kill_z_volume | create_pain_causing_volume |
        create_physics_volume | create_audio_volume | create_post_process_volume |
        create_nav_mesh_bounds_volume | create_nav_modifier_volume |
        create_level_instance | create_packed_level_actor |
        open_level_blueprint | add_level_blueprint_node
```

#### manage_geometry — 程序化网格
```
action: create_box | create_sphere | create_cylinder | create_cone |
        create_capsule | create_torus | create_plane | create_disc |
        create_stairs | create_spiral_stairs | create_ring | create_arch |
        create_pipe | create_ramp |
        boolean_union | boolean_subtract | boolean_intersection |
        extrude | inset | bevel | shell | revolve | chamfer |
        bend | twist | taper | noise_deform | smooth | relax |
        auto_uv | project_uv | unwrap_uv | pack_uv_islands |
        recalculate_normals | flip_normals |
        generate_collision | generate_lods | set_lod_screen_sizes |
        convert_to_nanite | convert_to_static_mesh |
        merge_vertices | weld_vertices | fill_holes |
        simplify_mesh | subdivide | remesh_uniform | remesh_voxel
```

#### manage_pcg — PCG 程序化内容
```
action: create_pcg_graph | create_pcg_subgraph | add_pcg_node |
        connect_pcg_pins | set_pcg_node_settings |
        add_landscape_data_node | add_spline_data_node | add_volume_data_node |
        add_surface_sampler | add_mesh_sampler | add_spline_sampler |
        add_bounds_modifier | add_density_filter | add_height_filter |
        add_slope_filter | add_transform_points | add_static_mesh_spawner |
        add_actor_spawner | execute_pcg_graph | set_pcg_partition_grid_size
```

---

### 🎮 游戏系统

#### animation_physics — 动画与物理
```
action: create_animation_blueprint | create_blend_space | create_blend_space_1d |
        create_blend_space_2d | create_blend_tree | create_state_machine |
        add_state | add_transition | set_transition_rules |
        create_control_rig | create_ik_rig | setup_ik |
        create_pose_library | create_animation_sequence | set_bone_key |
        create_montage | add_montage_section | set_section_timing |
        add_montage_notify | play_montage |
        setup_ragdoll | activate_ragdoll |
        setup_physics_simulation | configure_vehicle |
        create_skeleton | add_bone | remove_bone | set_bone_transform |
        create_socket | auto_skin_weights | normalize_weights | mirror_weights |
        create_physics_asset | add_physics_body | configure_physics_body |
        add_physics_constraint | create_morph_target | set_morph_target_deltas
```

#### manage_effect — Niagara 粒子特效
```
action: particle | niagara | debug_shape | spawn_niagara |
        create_niagara_system | create_niagara_emitter |
        add_niagara_module | connect_niagara_pins | set_niagara_parameter |
        add_emitter_to_system | set_emitter_properties |
        add_spawn_rate_module | add_spawn_burst_module |
        add_initialize_particle_module | add_force_module | add_velocity_module |
        add_color_module | add_sprite_renderer_module | add_mesh_renderer_module |
        add_collision_module | add_kill_particles_module |
        enable_gpu_simulation | get_niagara_info
```

#### manage_character — 角色系统
```
action: create_character_blueprint | configure_capsule_component |
        configure_mesh_component | configure_camera_component |
        configure_movement_speeds | configure_jump | configure_rotation |
        setup_mantling | setup_vaulting | setup_climbing |
        setup_sliding | setup_wall_running | setup_grappling |
        setup_footstep_system | configure_crouch | configure_sprint
```
关键参数: `parentClass`(Character|AICharacter), `skeletalMeshPath`, `walkSpeed`, `runSpeed`, `jumpHeight`, `gravityScale`, `springArmLength`

#### manage_combat — 战斗系统
```
action: create_weapon_blueprint | configure_weapon_mesh | set_weapon_stats |
        configure_hitscan | configure_projectile | configure_spread_pattern |
        configure_recoil_pattern | configure_aim_down_sights |
        create_projectile_blueprint | configure_projectile_movement |
        configure_projectile_collision | configure_projectile_homing |
        create_damage_type | setup_hitbox_component | setup_reload_system |
        setup_ammo_system | setup_attachment_system | setup_weapon_switching |
        configure_muzzle_flash | configure_tracer | configure_impact_effects |
        create_melee_trace | configure_combo_system | create_hit_pause |
        setup_parry_block_system | configure_weapon_trails |
        apply_damage | heal | create_shield | modify_armor
```
关键参数: `baseDamage`, `fireRate`, `range`, `spread`, `projectileSpeed`, `magazineSize`, `reloadTime`, `comboWindowTime`, `maxComboCount`

#### manage_ai — AI 系统
```
action: create_ai_controller | assign_behavior_tree | assign_blackboard |
        create_blackboard_asset | add_blackboard_key |
        create_behavior_tree | add_composite_node | add_task_node |
        add_decorator | add_service | configure_bt_node |
        create_eqs_query | add_eqs_generator | add_eqs_test |
        add_ai_perception_component | configure_sight_config |
        configure_hearing_config | configure_damage_sense_config |
        create_state_tree | add_state_tree_state | add_state_tree_transition |
        create_smart_object_definition | add_smart_object_slot |
        create_mass_entity_config | add_mass_spawner |
        set_blackboard_value | get_blackboard_value |
        run_behavior_tree | stop_behavior_tree | set_focus | clear_focus |
        configure_nav_mesh_settings | rebuild_navigation |
        create_nav_link_proxy | create_smart_link
```

#### manage_inventory — 物品系统
```
action: create_item_data_asset | set_item_properties | create_item_category |
        create_inventory_component | configure_inventory_slots |
        set_inventory_replication |
        create_pickup_actor | configure_pickup_interaction | configure_pickup_respawn |
        create_equipment_component | define_equipment_slots |
        configure_equipment_effects | configure_equipment_visuals |
        create_loot_table | add_loot_entry | configure_loot_drop |
        set_loot_quality_tiers |
        create_crafting_recipe | configure_recipe_requirements |
        create_crafting_station | add_crafting_component |
        configure_item_stacking | set_item_icon
```

#### manage_interaction — 交互系统
```
action: create_interaction_component | configure_interaction_trace |
        configure_interaction_widget | add_interaction_events |
        create_interactable_interface |
        create_door_actor | configure_door_properties |
        create_switch_actor | configure_switch_properties |
        create_chest_actor | configure_chest_properties |
        create_lever_actor |
        setup_destructible_mesh | configure_destruction_levels |
        create_trigger_actor | configure_trigger_events |
        configure_trigger_filter | configure_trigger_response
```

#### manage_gas — GAS 能力系统
```
action: add_ability_system_component | configure_asc |
        create_attribute_set | add_attribute | set_attribute_base_value |
        set_attribute_clamping |
        create_gameplay_ability | set_ability_tags | set_ability_costs |
        set_ability_cooldown | set_ability_targeting | add_ability_task |
        set_activation_policy | set_instancing_policy |
        create_gameplay_effect | set_effect_duration |
        add_effect_modifier | set_modifier_magnitude |
        add_effect_execution_calculation | add_effect_cue |
        set_effect_stacking | set_effect_tags |
        create_gameplay_cue_notify | configure_cue_trigger | set_cue_effects |
        add_tag_to_asset
```

---

### 🎵 实用工具

#### manage_audio — 音频系统
```
action: create_sound_cue | play_sound_at_location | play_sound_2d |
        create_audio_component | create_sound_mix | push_sound_mix |
        set_base_sound_mix | fade_sound_in | fade_sound_out |
        create_ambient_sound | create_reverb_zone |
        create_metasound | add_metasound_node | connect_metasound_nodes |
        set_metasound_default | add_mix_modifier |
        create_attenuation_settings | configure_distance_attenuation |
        configure_spatialization | configure_occlusion |
        create_dialogue_voice | create_dialogue_wave |
        create_reverb_effect | create_source_effect_chain |
        add_source_effect | create_submix_effect | get_audio_info
```

#### manage_sequence — Sequencer 过场动画
```
action: create | open | add_camera | add_actor | add_actors |
        remove_actors | get_bindings | play | pause | stop |
        set_playback_speed | add_keyframe | get_properties | set_properties |
        duplicate | rename | delete | list | get_metadata | set_metadata |
        add_spawnable_from_class | add_track | add_section |
        set_display_rate | set_tick_resolution | set_work_range | set_view_range |
        set_track_muted | set_track_solo | set_track_locked |
        list_tracks | remove_track | list_track_types
```

#### manage_networking — 多人网络
```
action: set_property_replicated | set_replication_condition |
        configure_net_update_frequency | configure_net_priority |
        set_net_dormancy | configure_replication_graph |
        create_rpc_function | configure_rpc_validation | set_rpc_reliability |
        set_owner | set_autonomous_proxy | check_has_authority |
        configure_net_cull_distance | set_always_relevant |
        configure_push_model | configure_client_prediction |
        configure_server_correction | configure_movement_prediction |
        configure_net_driver | configure_replicated_movement |
        create_input_action | create_input_mapping_context |
        add_mapping | remove_mapping | map_input_action |
        create_game_mode | create_game_state | create_player_controller |
        create_player_state | create_game_instance | create_hud_class |
        set_default_pawn_class | set_player_controller_class |
        configure_game_rules | setup_match_states | configure_round_system |
        configure_team_system | configure_scoring_system |
        configure_spawn_system | set_respawn_rules | configure_spectating |
        configure_local_session_settings | host_lan_server | join_lan_server |
        enable_voice_chat | configure_voice_settings | mute_player
```

---

## 常见任务模板

### 在编辑器中生成一个立方体
```json
{
  "name": "control_actor",
  "arguments": {
    "action": "spawn",
    "classPath": "StaticMeshActor",
    "actorName": "TestCube",
    "location": {"x": 500, "y": 0, "z": 200},
    "scale": {"x": 3, "y": 3, "z": 3}
  }
}
```

### 启动 PIE 并截图
```json
{"name": "control_editor", "arguments": {"action": "play"}}
{"name": "control_editor", "arguments": {"action": "screenshot", "width": 1920, "height": 1080}}
```

### 生成程序化几何体（球体）
```json
{
  "name": "manage_geometry",
  "arguments": {
    "action": "create_sphere",
    "actorName": "TestSphere",
    "radius": 100,
    "numSides": 32,
    "location": {"x": 0, "y": 0, "z": 100}
  }
}
```

### 执行控制台命令
```json
{
  "name": "control_editor",
  "arguments": {"action": "console_command", "command": "stat fps"}
}
```

### 检查 Actor 属性
```json
{
  "name": "inspect",
  "arguments": {
    "action": "inspect_object",
    "actorName": "TestCube"
  }
}
```

### 创建角色蓝图
```json
{
  "name": "manage_character",
  "arguments": {
    "action": "create_character_blueprint",
    "name": "BP_PlayerCharacter",
    "path": "/Game/Characters",
    "parentClass": "Character",
    "walkSpeed": 600,
    "runSpeed": 900,
    "jumpHeight": 450,
    "gravityScale": 1.0
  }
}
```

### 运行 Python 脚本
```json
{
  "name": "system_control",
  "arguments": {
    "action": "execute_python",
    "code": "import unreal; print(unreal.EditorLevelLibrary.get_all_level_actors())"
  }
}
```

## 注意事项

1. **字段名是驼峰式**: `classPath` 不是 `class_path`，`actorName` 不是 `actor_name`
2. **向量是对象不是数组**: `{"x": 1, "y": 2, "z": 3}` 不是 `[1, 2, 3]`
3. **SSE 响应**: 工具调用返回 `text/event-stream`，解析 `data:` 行获取 JSON
4. **每个 POST 独立连接**: 服务器每次响应后关闭连接，客户端需重建连接
5. **Session 超时 1 小时**: 长时间不操作需重新 initialize
6. **NullRHI 模式**: 编辑器以 `-NullRHI` 运行，截图通过 MCP 协议获取
