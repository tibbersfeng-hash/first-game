# Coding Standards

- All game code must include doc comments on public APIs
- Every system must have a corresponding architecture decision record in `docs/architecture/`
- Gameplay values must be data-driven (external config), never hardcoded
- All public methods must be unit-testable (dependency injection over singletons)
- Commits must reference the relevant design document or task ID
- **Commit messages**: Use Conventional Commits format — `feat:`, `fix:`, `chore:`, `docs:`, `test:`, `refactor:`. Reference the story or task ID in the body (e.g., `Story: EPIC-001-S02`).
- **Verification-driven development**: Write tests first when adding gameplay systems.
  For UI changes, verify with screenshots. Compare expected output to actual output
  before marking work complete. Every implementation should have a way to prove it works.

# Design Document Standards

- All design docs use Markdown
- Each mechanic has a dedicated document in `design/gdd/`
- Documents must include these 8 required sections:
  1. **Overview** -- one-paragraph summary
  2. **Player Fantasy** -- intended feeling and experience
  3. **Detailed Rules** -- unambiguous mechanics
  4. **Formulas** -- all math defined with variables
  5. **Edge Cases** -- unusual situations handled
  6. **Dependencies** -- other systems listed
  7. **Tuning Knobs** -- configurable values identified
  8. **Acceptance Criteria** -- testable success conditions
- Balance values must link to their source formula or rationale

# Testing Standards

## Test Evidence by Story Type

All stories must have appropriate test evidence before they can be marked Done:

| Story Type | Required Evidence | Location | Gate Level |
|---|---|---|---|
| **Logic** (formulas, AI, state machines) | Automated unit test — must pass | `tests/unit/[system]/` | BLOCKING |
| **Integration** (multi-system) | Integration test OR documented playtest | `tests/integration/[system]/` | BLOCKING |
| **Visual/Feel** (animation, VFX, feel) | Screenshot + lead sign-off | `production/qa/evidence/` | ADVISORY |
| **UI** (menus, HUD, screens) | Manual walkthrough doc OR interaction test | `production/qa/evidence/` | ADVISORY |
| **Config/Data** (balance tuning) | Smoke check pass | `production/qa/smoke-[date].md` | ADVISORY |

## MCP-Backed Visual Verification

> 通过 `ue-mcp-specialist` 远程操控 GPU 服务器上的 UE5.7 编辑器，对视觉/运行时改动
> 进行自动截图和数据抓取。这是 "验证驱动开发" 的视觉验证通道。

### 当 MCP 服务器可用时

| Story 类型 | MCP 验证方式 | 证据存储 |
|---|---|---|
| **Visual/Feel** | `control_editor(screenshot)` 多视角截图 | `production/qa/evidence/mcp-[story-id].png` |
| **UI** | Widget 编辑器截图 + `simulate_input` 交互 | `production/qa/evidence/mcp-[story-id]-ui.png` |
| **Logic/Integration** | PIE + `runtime_report` + `get_performance_stats` | `production/qa/evidence/mcp-[story-id]-runtime.md` |
| **Config/Data** | PIE 后 stat 验证 + 截图 | `production/qa/evidence/mcp-[story-id]-smoke.md` |

### 当 MCP 服务器不可用时

- 验证任务追加到 `production/qa/deferred-mcp-verifications.md`
- story 仍可标记 Done，但需在 MCP 上线后补验（story-done 时提醒）
- 不阻塞开发流程

### MCP 健康检查

```bash
curl -s -m 5 --noproxy '*' -X POST http://172.25.0.86:3000/mcp \
  -H "Content-Type: application/json" \
  -H "X-MCP-Capability-Token: c74a40cde871789c0b27984019fc58e4" \
  -d '{"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2025-03-26","capabilities":{},"clientInfo":{"name":"health-check","version":"1.0"}}}'
```

HTTP 200 + 返回 sessionId → 可用；其他 → 走降级策略。

## Automated Test Rules

- **Naming**: `[system]_[feature]_test.[ext]` for files; `test_[scenario]_[expected]` for functions
- **Determinism**: Tests must produce the same result every run — no random seeds, no time-dependent assertions
- **Isolation**: Each test sets up and tears down its own state; tests must not depend on execution order
- **No hardcoded data**: Test fixtures use constant files or factory functions, not inline magic numbers
  (exception: boundary value tests where the exact number IS the point)
- **Independence**: Unit tests do not call external APIs, databases, or file I/O — use dependency injection

## What NOT to Automate

- Visual fidelity (shader output, VFX appearance, animation curves)
- "Feel" qualities (input responsiveness, perceived weight, timing)
- Platform-specific rendering (test on target hardware, not headlessly)
- Full gameplay sessions (covered by playtesting, not automation)

## CI/CD Rules

- Automated test suite runs on every push to main and every PR
- No merge if tests fail — tests are a blocking gate in CI
- Never disable or skip failing tests to make CI pass — fix the underlying issue
- Engine-specific CI commands:
  - **Godot**: `godot --headless --script tests/gdunit4_runner.gd`
  - **Unity**: `game-ci/unity-test-runner@v4` (GitHub Actions)
  - **Unreal**: headless runner with `-nullrhi` flag
