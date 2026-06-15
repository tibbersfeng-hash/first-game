#!/usr/bin/env python3
"""
GDScript Test Validator — 静态验证测试文件结构
检查所有测试文件是否符合 GUT 约定：
1. 继承 gut/test.gd
2. 包含 before_each/after_each 生命周期
3. 所有 test_ 方法有断言调用
4. 测试覆盖所有核心 API 方法
"""

import os
import re
import sys
from pathlib import Path
from dataclasses import dataclass, field

PROJECT_ROOT = Path("/home/claude/.frontend/first-game")
TEST_DIR = PROJECT_ROOT / "tests" / "unit"
SRC_DIR = PROJECT_ROOT / "src"


@dataclass
class TestResult:
    file: str
    total_tests: int = 0
    test_names: list = field(default_factory=list)
    has_before_each: bool = False
    has_after_each: bool = False
    assertions_per_test: dict = field(default_factory=dict)
    errors: list = field(default_factory=list)


def parse_test_file(filepath: Path) -> TestResult:
    """Parse a GUT test file and extract test metadata."""
    result = TestResult(file=str(filepath.relative_to(PROJECT_ROOT)))
    content = filepath.read_text()

    # Check extends
    if 'extends "res://addons/gut/test.gd"' not in content:
        result.errors.append("Missing 'extends gut/test.gd'")

    # Find before_each / after_each
    result.has_before_each = "func before_each()" in content
    result.has_after_each = "func after_each()" in content

    # Find all test functions
    test_pattern = re.compile(r'func (test_\w+)\(\)')
    for match in test_pattern.finditer(content):
        test_name = match.group(1)
        result.test_names.append(test_name)
        result.total_tests += 1

    # Check assertions in each test
    assertion_funcs = [
        "assert_eq", "assert_ne", "assert_true", "assert_false",
        "assert_null", "assert_not_null", "assert_gt", "assert_lt",
        "assert_gte", "assert_lte", "assert_almost_eq",
    ]
    assertion_pattern = re.compile(r'\b(' + '|'.join(assertion_funcs) + r')\b')

    for match in test_pattern.finditer(content):
        test_name = match.group(1)
        start = match.end()
        # Find next func or end of file
        next_func = re.search(r'\nfunc ', content[start:])
        end = start + next_func.start() if next_func else len(content)
        test_body = content[start:end]
        assertions = assertion_pattern.findall(test_body)
        result.assertions_per_test[test_name] = len(assertions)
        if len(assertions) == 0:
            result.errors.append(f"  {test_name}: No assertions found!")

    return result


def check_api_coverage(test_file: Path, source_file: Path) -> list:
    """Check that all public methods in source file have corresponding tests."""
    errors = []
    if not source_file.exists():
        return [f"Source file not found: {source_file}"]

    src_content = source_file.read_text()
    test_content = test_file.read_text()

    # Find public method definitions in source
    public_methods = re.findall(r'func (\w+)\(', src_content)
    # Filter out private methods (starting with _)
    public_methods = [m for m in public_methods if not m.startswith('_')]

    # Check each public method has at least one test referencing it
    untested = []
    for method in public_methods:
        # Skip _ready, _process, _physics_process, _init etc.
        if method in ('_ready', '_process', '_physics_process', '_init',
                       '_enter_tree', '_exit_tree', '_unhandled_input'):
            continue
        if method not in test_content:
            untested.append(method)

    if untested:
        errors.append(f"  Untested public methods: {', '.join(untested)}")

    return errors


# Mapping: test file -> source file
TEST_TO_SOURCE = {
    "tests/unit/utils/frame_timer_test.gd": "src/utils/frame_timer.gd",
    "tests/unit/core/input/buffered_input_test.gd": "src/core/input/buffered_input.gd",
    "tests/unit/core/input/input_buffer_test.gd": "src/core/input/input_buffer.gd",
    "tests/unit/core/components/health_component_test.gd": "src/core/components/health_component.gd",
    "tests/unit/core/components/energy_component_test.gd": "src/core/components/energy_component.gd",
    "tests/unit/core/state_machine/state_machine_test.gd": "src/core/state_machine/state_machine.gd",
    "tests/unit/gameplay/combat/combo_manager_test.gd": "src/gameplay/combat/combo_manager.gd",
    "tests/unit/infrastructure/game_manager_test.gd": "src/infrastructure/autoloads/game_manager.gd",
}


def main():
    print("=" * 60)
    print("GDScript Test Validator — 静态验证")
    print("=" * 60)

    total_tests = 0
    total_errors = 0
    total_warnings = 0

    for test_rel, src_rel in TEST_TO_SOURCE.items():
        test_path = PROJECT_ROOT / test_rel
        src_path = PROJECT_ROOT / src_rel

        print(f"\n📋 {test_rel}")

        if not test_path.exists():
            print(f"   ❌ Test file not found!")
            total_errors += 1
            continue

        result = parse_test_file(test_path)

        # Print test count
        print(f"   ✅ {result.total_tests} test functions found")
        total_tests += result.total_tests

        # Check before_each / after_each
        if not result.has_before_each:
            print(f"   ⚠️  Missing before_each()")
            total_warnings += 1
        if not result.has_after_each:
            print(f"   ⚠️  Missing after_each()")
            total_warnings += 1

        # Print test details
        for name in result.test_names:
            count = result.assertions_per_test.get(name, 0)
            icon = "✅" if count > 0 else "❌"
            print(f"   {icon} {name} ({count} assertions)")

        # Check API coverage
        coverage_errors = check_api_coverage(test_path, src_path)
        for err in coverage_errors:
            print(f"   ⚠️  {err}")
            total_warnings += 1

        # Print errors
        for err in result.errors:
            print(f"   ❌ {err}")
            total_errors += 1

    # Summary
    print("\n" + "=" * 60)
    print(f"📊 Summary: {total_tests} tests, {total_errors} errors, {total_warnings} warnings")

    # Count assertions
    all_assertions = 0
    for test_rel in TEST_TO_SOURCE:
        test_path = PROJECT_ROOT / test_rel
        if test_path.exists():
            content = test_path.read_text()
            assertion_funcs = [
                "assert_eq", "assert_ne", "assert_true", "assert_false",
                "assert_null", "assert_not_null", "assert_gt", "assert_lt",
                "assert_gte", "assert_lte", "assert_almost_eq",
            ]
            for func in assertion_funcs:
                all_assertions += len(re.findall(rf'\b{func}\b', content))

    print(f"📊 Total assertions: {all_assertions}")
    print(f"📊 Test files: {len(TEST_TO_SOURCE)}")
    print(f"📊 Source files covered: {len(TEST_TO_SOURCE)}")

    if total_errors > 0:
        print(f"\n❌ FAILED — {total_errors} errors found")
        sys.exit(1)
    else:
        print(f"\n✅ PASSED — All test files are structurally valid")
        sys.exit(0)


if __name__ == "__main__":
    main()
