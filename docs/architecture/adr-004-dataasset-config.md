# ADR-004: DataAsset-Based Data Configuration

- **Status**: Accepted
- **Date**: 2026-06-17
- **Engine**: Unreal Engine 5.6

## Context

角色属性、攻击参数、敌人配置等游戏数据需要从代码中分离，支持策划调整。Godot 方案使用 Resource (.tres) 文件。

## Decision

**使用 `UPrimaryDataAsset` 和 `UDataAsset` 作为游戏数据载体。**

### 资产类型

```
UCharacterDataAsset (UPrimaryDataAsset)
├── CharacterId: FName
├── MaxHealth, MaxEnergy, MoveSpeed, JumpForce
├── LightAttacks: TArray<FAttackMoveData>
├── HeavyAttack, SpecialMove, DodgeMove
└── HitStunDuration, KnockbackResistance

FAttackMoveData (USTRUCT)
├── MoveName, Damage
├── StartupFrames, ActiveFrames, RecoveryFrames
├── HitStopFrames, Knockback, LaunchForce
```

### 数据流

```
Content/ (编辑器创建 .uasset)
    → UCharacterDataAsset (加载到内存)
    → UCombatDataSubsystem::RegisterCharacterData()
    → APlayerCharacter::InitializeCharacter(DataAsset)
```

### 访问方式

- **编辑器**: 右键 Content Browser → Miscellaneous → Data Asset → 选择 CharacterDataAsset
- **C++**: `CombatDataSubsystem->GetCharacterData("Huikong")`
- **Blueprint**: `Get Character Data` 节点

## Consequences

- ✅ 策划可在编辑器中直接调整数值
- ✅ 支持热重载（Development 模式）
- ✅ 类型安全，编译期检查
- ✅ 可版本控制（.uasset 为二进制，可用 Git LFS）
- ⚠️ 批量修改数值不如 JSON/CSV 方便（可用 DataTable 补充）
