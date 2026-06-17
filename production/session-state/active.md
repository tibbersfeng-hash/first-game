# Session State — 格斗萌主

*Last updated: 2026-06-17*

## Current Phase
MVP Prototyping — UE5 Code Compile ✅

## Engine
- **Engine**: Unreal Engine 5.7.4
- **Language**: C++ (核心) + Blueprint (内容)
- **Approach**: 2.5D (固定侧视相机)
- **Project Path (local)**: `src_ue5/`
- **Project Path (cloud)**: `/root/project/FirstGame/`
- **Source**: 48 files (25 .h + 23 .cpp), 55 total
- **Cloud Server**: connect.westd.seetacloud.com:51280 (RTX 5090, 754GB RAM)

## Completed — All Phases

### Design (preserved from Godot phase)
- [x] 11 个 GDD 文档
- [x] 角色设计定稿（慧空/糖糖/小鬼丸）
- [x] 资产清单 + 资产规格（ASSET-001~036）
- [x] AI 出图管线 v2（wan2.7-image-pro + qwen-image-edit-max）

### UE5 代码 (src_ue5/)
- [x] **Phase 1: 项目搭建** — .uproject, Build.cs, Target.cs, 配置, .gitignore
- [x] **Phase 2: 核心架构** — 7 ADR 重写, Enhanced Input, 架构文档
- [x] **Phase 3: 战斗系统** — GAS CombatAbility, ComboManager, HitStopManager, HitBox/HurtBox
- [x] **Phase 4: 敌人 & AI** — BaseEnemy, EnemyAIController, 3×BT Task 节点
- [x] **Phase 5: 关卡 & HUD** — DungeonRoom, DungeonFlow, HUDWidget, DamageNumberWidget
- [x] **代码审查** — 多次编译错误修复

### UE5 编译修复记录
- [x] 添加 FirstGame.Target.cs + FirstGameEditor.Target.cs
- [x] .uproject: 移除误作为插件的内置模块 (GameplayTags, GameplayTasks, AIModule)
- [x] 修复 HitCameraShake UPROPERTY 放在 delegate 声明上的错误
- [x] 重命名 bIsActive → bHitBoxEnabled/bHitStopEnabled (避免与 UActorComponent 冲突)
- [x] CombatAbility: 移除不存在的 ActivationPolicy, 修复 lambda 捕获
- [x] BaseEnemy/PlayerCharacter: TakeDamage → ReceiveHitDamage (避免隐藏 APawn::TakeDamage)
- [x] HitCameraShake: 移除 UE5.7 中不存在的 UpdateCameraShake/IsFinished override
- [x] FDamageNumberData: 添加参数化构造函数
- [x] AudioManager: PlaySoundAttached → PlaySoundAtLocation (UE5 已移除旧 API)
- [x] BTTaskChasePlayer: StopMovement() 在 AAIController 上而非 APawn
- [x] BTTaskDetectPlayer: FHitResult 临时变量不能绑定到非 const 引用
- [x] DamageNumberWidget: FObjectFinder 不存在 → 移除自定义字体设置
- [x] WaveManager: ANSI 字符串 → TEXT() 宏, OnEnemyDied 签名修正
- [x] DungeonFlow: 修正 #include 路径
- [x] SignalBusFunctionLibrary: UObject::GetGameInstance() → GetWorld()->GetGameInstance()
- [x] HitBoxComponent: ClientPlayCameraShake → ClientStartCameraShake (UE5.7 API)
- [x] HitCameraShake: 添加 FObjectInitializer 构造 (UCameraShakeBase 无默认构造)
- [x] 重建引擎 BuildRules (UE5Rules.dll) — 修改 DynamicCompilation.cs 后重新编译 UBT
- [x] 手动创建 FirstGameEditor.target + UnrealEditor.modules receipt 文件

### 文档
- [x] 7 个 ADR (UE5 版本)
- [x] Architecture Document (UE5 版本)
- [x] Control Manifest (UE5 版本)
- [x] technical-preferences.md (UE5 版本)
- [x] Sprite 生成管线文档

## 编译状态
- ✅ Editor 模块: `libUnrealEditor-FirstGame.so` 编译+链接成功
- ✅ 模块加载: Editor 成功识别并加载 FirstGame 模块
- ⚠️ VNC 渲染: VNC 不支持 GLX/OpenGL，需要真实 GPU 显示环境
- ❌ Game 目标: 链接器缺少引擎启动模块 (InstalledBuild 限制)

## Next Steps
1. 通过真实 GPU 显示环境（物理机/外部显示器）启动 Editor 验证
2. 或配置 VNC + VirtualGL/TurboVNC 支持 OpenGL
3. 创建 Blueprint 资产（DataAsset、BehaviorTree、AnimMontage）
4. 导入 AI 生成的角色 sprite
5. 创建测试 Level 验证基础玩法
