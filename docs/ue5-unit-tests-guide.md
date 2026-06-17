# UE5 单元测试指南

> **项目**: 格斗萌主 (FirstGame)
> **框架**: UE5 Automation Framework
> **测试模块**: FirstGameTests

---

## 测试文件清单

| 文件 | 测试数 | 覆盖系统 |
|------|--------|----------|
| `ComboManagerTests.cpp` | 7 | 连招管理 |
| `HitStopManagerTests.cpp` | 4 | 顿帧管理 |
| `CharacterDataAssetTests.cpp` | 5 | 角色数据资产 |
| `WaveManagerTests.cpp` | 5 | 波次管理 |
| `LootItemTests.cpp` | 5 | 掉落系统 |
| `SignalBusTests.cpp` | 4 | 事件总线 |
| `CombatAbilityTests.cpp` | 4 | 战斗技能 |
| `BaseEnemyTests.cpp` | 4 | 敌人基础 |
| `HitBoxComponentTests.cpp` | 4 | 攻击判定 |
| `AudioManagerTests.cpp` | 3 | 音频管理 |
| `HitCameraShakeTests.cpp` | 4 | 屏幕特效 |
| `IntegrationTests.cpp` | 6 | 跨系统集成 |
| **总计** | **55** | |

---

## 运行测试

### 方法 1: UE5 编辑器内运行

1. 打开 UE5 编辑器
2. 菜单 **Window → Developer Tools → Automation**
3. 在测试树中找到 `FirstGame.*`
4. 选择要运行的测试组或全选
5. 点击 **Start Tests**

### 方法 2: 命令行运行

```bash
# 运行所有 FirstGame 测试
./UE5Editor.sh FirstGame.uproject -ExecCmds="Automation RunTests FirstGame" -unattended -stdout -fullstdoutlogoutput -log

# 运行特定测试组
./UE5Editor.sh FirstGame.uproject -ExecCmds="Automation RunTests FirstGame.Combat" -unattended -stdout

# 运行特定测试
./UE5Editor.sh FirstGame.uproject -ExecCmds="Automation RunTests FirstGame.Combat.ComboManager" -unattended -stdout
```

### 方法 3: CI/CD 集成

```bash
#!/bin/bash
# run_tests.sh
UE5_BIN="/path/to/UE5/Engine/Binaries/Linux/UnrealEditor"
PROJECT="/path/to/first-game/src_ue5/FirstGame.uproject"

"$UE5_BIN" "$PROJECT" \
    -ExecCmds="Automation RunTests FirstGame; Automation Quit" \
    -unattended \
    -stdout \
    -fullstdoutlogoutput \
    -log \
    -ReportOutputPath="./TestResults" \
    -NullRHI \
    -nosound

# Check exit code
if [ $? -eq 0 ]; then
    echo "All tests passed!"
else
    echo "Some tests failed!"
    exit 1
fi
```

---

## 测试分类

### Unit Tests (纯逻辑测试)
不需要 UE5 World 上下文，可直接运行：
- `FComboManagerInitTest`
- `FComboManagerFirstHitTest`
- `FComboManagerContinuationTest`
- `FComboManagerMaxClampTest`
- `FComboManagerResetTest`
- `FComboManagerNextIndexTest`
- `FComboManagerWindowConfigTest`
- `FHitStopManagerInitTest`
- `FHitStopManagerConfigTest`
- `FHitStopManagerStackingLogicTest`
- `FCharacterDataAssetDefaultsTest`
- `FCharacterDataAssetPrimaryIdTest`
- `FCharacterDataAssetCustomValuesTest`
- `FAttackMoveDataDefaultsTest`
- `FAttackMoveDataCustomTest`
- `FLootTypeValuesTest`
- `FLootDropConfigDefaultsTest`
- `FLootDropConfigCustomTest`
- `FLootTableCreationTest`
- `FLootDropChanceBoundaryTest`
- `FVolumeClampTest`
- `FComboSoundVolumeTest`
- `FHitCameraShakeDefaultsTest`
- `FHitCameraShakeFinishedTest`
- `FHitCameraShakeDecayTest`
- `FScreenFlashConfigTest`
- `F2DPlaneConstraintTest`

### Integration Tests (需要 World 上下文)
需要 UE5 World 才能完整运行：
- `FWaveManagerInitTest` — 需要 SpawnActor
- `FWaveManagerSetupTest` — 需要 SpawnActor
- `FWaveManagerEmptyWavesTest` — 需要 SpawnActor + Timer
- `FCombatFlowIntegrationTest` — 需要 ComboManager + HitStopManager
- `FDamageNumberFlowTest` — 纯数据结构测试
- `FCharacterDataToCombatTest` — 数据流测试
- `FWaveLootIntegrationTest` — 数据流测试

### Structural Tests (架构验证)
验证编译和类型正确性：
- `FSignalBusDelegatesTest`
- `FDamageNumberDataDefaultsTest`
- `FDamageNumberDataCustomTest`
- `FSignalBusEventFlowTest`
- `FCombatAbilityDefaultsTest`
- `FCombatAbilityFrameTimingTest`
- `FCombatAbilityHeavyTimingTest`
- `FCombatAbilityDamageConfigTest`
- `FBaseEnemyDefaultStateTest`
- `FBaseEnemyStateMachineTest`
- `FBaseEnemyAggroLogicTest`
- `FEnemyTypesConfigTest`
- `FHitBoxComponentConfigTest`
- `FHitBoxComponentKnockbackTest`
- `FHitBoxActivationLogicTest`
- `FHitBoxSelfHitPreventionTest`
- `FEAudioCategoryValuesTest`
- `FHitStopManagerRequestTest`

---

## 添加新测试

1. 在 `Source/FirstGameTests/Private/Tests/` 创建新的 `.cpp` 文件
2. 使用 `IMPLEMENT_SIMPLE_AUTOMATION_TEST` 宏定义测试
3. 测试命名格式: `FirstGame.<Category>.<Subsystem>.<TestName>`
4. 使用 `TestEqual`, `TestTrue`, `TestFalse`, `TestNotNull`, `TestNull` 进行断言

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMyNewTest,
    "FirstGame.Category.MySubsystem.MyTest",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMyNewTest::RunTest(const FString& Parameters)
{
    // 测试代码
    TestEqual("Expected value", actual, expected);
    return true;
}
```

---

## 测试覆盖率目标

| 系统 | 当前覆盖 | 目标 |
|------|----------|------|
| ComboManager | ✅ 100% | 100% |
| HitStopManager | ✅ 90% | 100% (Tick 需集成测试) |
| CharacterDataAsset | ✅ 100% | 100% |
| WaveManager | ⚠️ 70% | 90% (需 World 集成测试) |
| LootItem/LootTable | ✅ 90% | 100% |
| SignalBus | ✅ 80% | 90% (需 World 集成测试) |
| CombatAbility | ⚠️ 60% | 80% (需 GAS 集成测试) |
| BaseEnemy | ⚠️ 60% | 80% (需 World 集成测试) |
| HitBoxComponent | ⚠️ 70% | 90% (需 World 集成测试) |
| AudioManager | ⚠️ 50% | 80% (需 World 集成测试) |
| HitCameraShake | ✅ 90% | 100% |
