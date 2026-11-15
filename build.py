#!/usr/bin/env python3
import os
import sys
import argparse
import subprocess
import multiprocessing

def get_default_jobs():
    try:
        return str(multiprocessing.cpu_count())
    except Exception:
        return "1"

def main():
    # Workspace setup
    workspace = os.environ.get("WORKSPACE", "/workspace")
    if os.path.exists(workspace):
        os.chdir(workspace)

    parser = argparse.ArgumentParser(
        description="Build script",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""Examples:
  build.py
  build.py pc64 --configure-only
  build.py pc64 --build-only --jobs 8
  build.py pc64 --cmake-arg -DMY_OPTION=ON
"""
    )

    parser.add_argument(
        "target_pos",
        nargs="?",
        default=os.environ.get("BUILD_TARGET", "pc64"),
        help="Windows x86_64 build (default: pc64)"
    )
    parser.add_argument(
        "--target",
        dest="target_opt",
        help="Select a build target"
    )
    parser.add_argument(
        "--build-dir",
        help="Override the target's build directory"
    )
    parser.add_argument(
        "--build-type",
        default=os.environ.get("BUILD_TYPE", "Release"),
        help="CMake build type (default: Release)"
    )
    parser.add_argument(
        "--jobs",
        default=os.environ.get("BUILD_JOBS", get_default_jobs()),
        help="Parallel build jobs (default: nproc equivalent)"
    )
    parser.add_argument(
        "--configure-only",
        action="store_true",
        help="Configure CMake, but do not compile"
    )
    parser.add_argument(
        "--build-only",
        action="store_true",
        help="Compile an already-configured build directory"
    )
    parser.add_argument(
        "--no-deps",
        action="store_true",
        help="Skip tools/build/deps.py"
    )
    parser.add_argument(
        "--no-compile-commands",
        action="store_true",
        help="Do not export compile_commands.json"
    )
    parser.add_argument(
        "--cmake-arg",
        action="append",
        default=[],
        help="Append an argument to the CMake configure command"
    )

    args = parser.parse_args()

    target = args.target_opt if args.target_opt else args.target_pos

    if args.configure_only and args.build_only:
        print("ERROR: --configure-only and --build-only are mutually exclusive.", file=sys.stderr)
        sys.exit(2)

    # Target definitions
    if target == "pc64":
        cmake_source_dir = "tools/pc"
        toolchain_file = os.path.join(os.getcwd(), "tools/pc/mingw-x86_64.cmake")
        build_dir = args.build_dir if args.build_dir else "build/pc64"
    else:
        print(f"ERROR: Unsupported target '{target}'.", file=sys.stderr)
        print("Add its CMake configuration to the target definitions in the script.", file=sys.stderr)
        sys.exit(2)

    cmake_lists_path = os.path.join(cmake_source_dir, "CMakeLists.txt")
    if not os.path.isfile(cmake_lists_path):
        print(f"ERROR: CMake source not found: {cmake_lists_path}", file=sys.stderr)
        sys.exit(2)

    export_compile_commands = not args.no_compile_commands
    run_deps = not args.no_deps

    if not args.build_only:
        if run_deps:
            deps_script = "tools/build/deps.py"
            if os.path.isfile(deps_script):
                subprocess.run(["python3", deps_script, "build/deps"], check=True)
            else:
                print("ERROR: tools/build/deps.py not found (use --no-deps to skip).", file=sys.stderr)
                sys.exit(2)

        configure_args = [
            "cmake",
            "-S", cmake_source_dir,
            "-B", build_dir,
            "-G", "Ninja",
            f"-DCMAKE_BUILD_TYPE={args.build_type}"
        ]

        if toolchain_file:
            configure_args.append(f"-DCMAKE_TOOLCHAIN_FILE={toolchain_file}")

        if export_compile_commands:
            configure_args.append("-DCMAKE_EXPORT_COMPILE_COMMANDS=ON")

        configure_args.extend(args.cmake_arg)
        subprocess.run(configure_args, check=True)

    if not args.configure_only:
        build_ninja_path = os.path.join(build_dir, "build.ninja")
        if not os.path.isfile(build_ninja_path):
            print(f"ERROR: No configured Ninja build found in '{build_dir}'.", file=sys.stderr)
            print("Run without --build-only first.", file=sys.stderr)
            sys.exit(2)

        build_args = [
            "cmake",
            "--build", build_dir,
            "--parallel", str(args.jobs)
        ]
        subprocess.run(build_args, check=True)

    compile_commands = os.path.join(build_dir, "compile_commands.json")
    if export_compile_commands and os.path.isfile(compile_commands):
        cwd = "./" # Needs to be a relative link
        target_compile_commands = os.path.join(cwd, compile_commands)
        workspace_link = os.path.join(cwd, "compile_commands.json")

        if os.path.lexists(workspace_link):
            os.remove(workspace_link)
        os.symlink(target_compile_commands, workspace_link)

        print(f"Compile commands: {target_compile_commands}")
        print(f"Workspace link:    {workspace_link}")

    if not args.configure_only:
        print()
        print(f"Build complete: target={target} build_dir={build_dir}")

if __name__ == "__main__":
    main()
