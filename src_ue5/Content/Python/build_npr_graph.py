"""
Build NPR Material Graph via MCP Python execution
Creates: 2-tone shadow + Rim Light material
"""
import unreal

# Load the material
mat_path = "/Game/Materials/M_NPR_Parent"
mat = unreal.load_asset(mat_path)
if not mat:
    print("ERROR: Material not found")
else:
    print("Material loaded:", mat.get_name())

    # Get editor-only data
    eo = mat.get_editor_only_data()
    if not eo:
        print("ERROR: No editor-only data")
    else:
        # Clear existing expressions
        mat.get_expression_collection().clear_expressions()

        # 1) BaseColor parameter
        base_color = unreal.MaterialExpressionVectorParameter(mat)
        base_color.parameter_name = "BaseColor"
        base_color.default_value = unreal.LinearColor(0.8, 0.6, 0.4, 1.0)
        mat.get_expression_collection().add_expression(base_color)

        # 2) ShadowColor parameter
        shadow_color = unreal.MaterialExpressionVectorParameter(mat)
        shadow_color.parameter_name = "ShadowColor"
        shadow_color.default_value = unreal.LinearColor(0.3, 0.15, 0.2, 1.0)
        mat.get_expression_collection().add_expression(shadow_color)

        # 3) RimColor parameter
        rim_color = unreal.MaterialExpressionVectorParameter(mat)
        rim_color.parameter_name = "RimColor"
        rim_color.default_value = unreal.LinearColor(1.0, 0.9, 0.7, 1.0)
        mat.get_expression_collection().add_expression(rim_color)

        # 4) ShadowThreshold parameter
        shadow_thresh = unreal.MaterialExpressionScalarParameter(mat)
        shadow_thresh.parameter_name = "ShadowThreshold"
        shadow_thresh.default_value = 0.3
        mat.get_expression_collection().add_expression(shadow_thresh)

        # 5) LightDirection constant
        light_dir = unreal.MaterialExpressionConstant3Vector(mat)
        light_dir.constant = unreal.LinearColor(0.5, 0.3, 0.8, 1.0)
        mat.get_expression_collection().add_expression(light_dir)

        # 6) VertexNormalWS
        vert_normal = unreal.MaterialExpressionVertexNormalWS(mat)
        mat.get_expression_collection().add_expression(vert_normal)

        # 7) NdotL = Dot(VertexNormal, LightDir)
        ndotl = unreal.MaterialExpressionDotProduct(mat)
        ndotl.a = vert_normal
        ndotl.b = light_dir
        mat.get_expression_collection().add_expression(ndotl)

        # 8) ShadowStep = Step(Threshold, NdotL)
        shadow_step = unreal.MaterialExpressionStep(mat)
        shadow_step.a = shadow_thresh
        shadow_step.b = ndotl
        mat.get_expression_collection().add_expression(shadow_step)

        # 9) ColorLerp = Lerp(ShadowColor, BaseColor, ShadowStep)
        color_lerp = unreal.MaterialExpressionLinearInterpolate(mat)
        color_lerp.a = shadow_color
        color_lerp.b = base_color
        color_lerp.alpha = shadow_step
        mat.get_expression_collection().add_expression(color_lerp)

        # 10) RimScale constant (0.3)
        rim_scale = unreal.MaterialExpressionConstant(mat)
        rim_scale.r = 0.3
        mat.get_expression_collection().add_expression(rim_scale)

        # 11) RimContrib = RimColor * RimScale
        rim_contrib = unreal.MaterialExpressionMultiply(mat)
        rim_contrib.a = rim_color
        rim_contrib.b = rim_scale
        mat.get_expression_collection().add_expression(rim_contrib)

        # 12) FinalColor = ColorLerp + RimContrib
        final_add = unreal.MaterialExpressionAdd(mat)
        final_add.a = color_lerp
        final_add.b = rim_contrib
        mat.get_expression_collection().add_expression(final_add)

        # Connect to EmissiveColor output
        eo.emissive_color = final_add

        # Set material properties
        mat.set_shading_model(unreal.MaterialShadingModel.MSM_UNLIT)
        mat.b_used_with_static_lighting = False
        mat.b_used_with_skeletal_mesh = True

        # Save
        unreal.EditorAssetLibrary.save_asset(mat_path)

        expr_count = mat.get_expression_collection().expressions.__len__()
        print(f"SUCCESS: NPR material built with {expr_count} expressions")
        print("ShadingModel: Unlit")
        print("Output: EmissiveColor")
