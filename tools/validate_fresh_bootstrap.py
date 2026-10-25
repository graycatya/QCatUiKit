#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Validate a clean source snapshot using Qt Creator's no-console Windows environment."""
import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--qt-prefix", required=True)
    parser.add_argument("--vsdevcmd", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--qtcreator-helper", required=True)
    parser.add_argument("--managed-python", action="store_true")
    parser.add_argument("--resume", action="store_true", help="Resume an existing snapshot, preserving previous logs")
    args = parser.parse_args()
    if os.name != "nt":
        parser.error("This validation reproduces Qt Creator/MSVC on Windows")
    repo = Path(__file__).resolve().parents[1]
    output = Path(args.output).resolve()
    source = output / "fresh clone source"
    if args.resume:
        if not source.is_dir():
            parser.error("--resume requires a previous source snapshot")
        print("Resuming existing snapshot:", source, flush=True)
    else:
        if output.exists():
            parser.error("--output must be a new directory; use --resume for an existing snapshot")
        output.mkdir(parents=True)
        source.mkdir()
        names = subprocess.check_output(["git", "-c", "safe.directory=" + repo.as_posix(),
            "-C", str(repo), "ls-files", "--cached", "--others", "--exclude-standard", "-z"],
            stderr=subprocess.DEVNULL).decode("utf-8").split("\0")
        allowed = {"cmake", "src", "examples", "tests", "adapters", "LICENSES", "tools", "skills"}
        count = 0
        for name in sorted(set(names)):
            if not name:
                continue
            relative = Path(name)
            if len(relative.parts) > 1 and relative.parts[0] not in allowed:
                continue
            if any(p in {"build", "CMakeFiles", ".qtcreator", ".git", "__pycache__"} or p.startswith("build-")
                   for p in relative.parts):
                continue
            original = repo / relative
            if original.is_file():
                target = source / relative
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(original, target)
                count += 1
        assert not any((source / name).exists() for name in ("3rdparty", "3rdparty-download", "3rdparty-tools"))
        print("Fresh source snapshot:", count, "files, no dependency caches", flush=True)
    creationflags = subprocess.CREATE_NO_WINDOW
    # Qt Creator supplies the selected kit's developer environment, but it does
    # not give cl.exe an interactive console/codepage. Reproduce both facts.
    setup = output / "capture-vsenv.cmd"
    setup.write_text('@echo off\ncall "' + str(Path(args.vsdevcmd).resolve()) + '" -arch=x64 >nul\nif errorlevel 1 exit /b 1\nset\n', encoding="utf-8")
    result = subprocess.run(["cmd", "/d", "/c", str(setup)], stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE, creationflags=creationflags, check=True)
    env = dict(os.environ)
    for line in result.stdout.decode("mbcs", errors="replace").splitlines():
        if "=" in line and not line.startswith("="):
            key, value = line.split("=", 1)
            env[key] = value
    for name in ("QT_QUICK_BACKEND", "QT_QPA_PLATFORM", "QML_IMPORT_PATH", "QML2_IMPORT_PATH"):
        env.pop(name, None)
    qt = Path(args.qt_prefix).resolve()
    cmake = shutil.which("cmake")
    ninja = shutil.which("ninja")
    if not cmake or not ninja:
        parser.error("CMake and Ninja must be available, as provided by the Qt kit")
    def run(command, log, extra_env=None, capture=False):
        print("Running", log, flush=True)
        current_env = dict(env)
        if extra_env:
            current_env.update(extra_env)
        log_path = output / log
        if log_path.exists():
            previous = log_path.with_suffix(log_path.suffix + ".previous")
            index = 1
            while previous.exists():
                previous = log_path.with_suffix(log_path.suffix + ".previous-" + str(index))
                index += 1
            log_path.rename(previous)
        with log_path.open("wb") as stream:
            completed = subprocess.run([str(x) for x in command], env=current_env,
                cwd=output, stdout=stream, stderr=subprocess.STDOUT, creationflags=creationflags)
        if completed.returncode:
            print((output / log).read_text(encoding="utf-8", errors="replace")[-8000:], flush=True)
            raise RuntimeError("Failed: " + log)
        if capture:
            return (output / log).read_text(encoding="utf-8", errors="replace").strip()
    report = {"source": str(source), "no_console": True, "resumed": args.resume, "configs": {}}
    for config in ("Debug", "Release"):
        build = output / config
        helper = build / ".qtcreator/cmake-helper"
        shutil.copytree(Path(args.qtcreator_helper).resolve().parent, helper, dirs_exist_ok=True)
        command = [cmake, "-S", source, "-B", build, "-GNinja", "-DCMAKE_BUILD_TYPE=" + config,
            "-DCMAKE_PREFIX_PATH=" + qt.as_posix(), "-DCMAKE_MAKE_PROGRAM=" + ninja,
            "-DCMAKE_PROJECT_INCLUDE_BEFORE=" + (helper / "qtcreator-project.cmake").as_posix(),
            "-DQT_CREATOR_ENABLE_MAINTENANCE_TOOL_PROVIDER=ON", "-DQT_CREATOR_ENABLE_PACKAGE_MANAGER_SETUP=ON",
            "-DQT_ENABLE_QML_DEBUG=ON", "-DCMAKE_COLOR_DIAGNOSTICS=ON"]
        if args.managed_python:
            command.append("-DQCATUIKIT_BOOTSTRAP_PYTHON=ON")
        run(command, config + "-configure.log")
        cache = (build / "CMakeCache.txt").read_text(encoding="utf-8")
        assert "QCATUIKIT_ENABLE_THORVG:BOOL=ON" in cache
        assert "QCATUIKIT_THORVG_SOURCE_DIR:PATH=\n" in cache
        run([cmake, "--build", build, "--parallel", "5"], config + "-build.log")
        run([cmake, "--build", build, "--target", "QCatUiKit_qmllint", "qcat-gallery_qmllint"], config + "-lint.log")
        run([ninja, "-C", build, "-t", "recompact"], config + "-recompact.log")
        run(["ctest", "--test-dir", build, "-R", "^(gallery_smoke|qml_contracts|vector_contracts)$", "--output-on-failure"], config + "-tests.log")
        runtime = {"PATH": str(build / "bin") + os.pathsep + str(qt / "bin") + os.pathsep + env["PATH"],
                   "QSG_RHI_BACKEND": "d3d11"}
        run([build / "bin/qcat-gallery.exe", "--page", "vector-canvas", "--screenshot", output / (config + "-canvas.png")],
            config + "-gallery.log", runtime)
        for path in (build / "qml/QCatUiKit/QCatUiKit.qmltypes",
                     build / "qml/QCatUiKit/Gallery/qcat-gallery-ui.qmltypes"):
            assert path.stat().st_size > 0
        deps = run([ninja, "-C", build, "-t", "deps", "src/uikit/CMakeFiles/QCatUiKit.dir/qcatuikit_qmltyperegistrations.cpp.obj"],
                   config + "-deps.log", capture=True)
        assert "qcatvectoritem.h" in deps and "#deps 0," not in deps
        meson_builds = list((build / "third_party").glob("*/meson/" + config))
        assert len(meson_builds) == 1
        project_info = json.loads((meson_builds[0] / "meson-info/intro-projectinfo.json").read_text(encoding="utf-8"))
        assert project_info["version"] == "1.1.3", project_info
        meson_deps = run([ninja, "-C", meson_builds[0], "-t", "deps",
            "src/libthorvg-1.a.p/renderer_gpu_engine_qrhi_qcatthorvgqrhirenderer.cpp.obj"],
            config + "-meson-deps.log", capture=True)
        assert "qcatthorvgqrhirenderer_p.h" in meson_deps and "#deps 0," not in meson_deps
        report["configs"][config] = "configure/build/lint/metadata/tests/native-canvas passed"
    # Real include dependency test, including a non-ASCII directory. A stale
    # object after changing this header would reproduce the original failure.
    incremental = output / "incremental"
    run([cmake, "-S", source / "tests/auto/cmake/msvc-incremental", "-B", incremental, "-GNinja",
         "-DCMAKE_BUILD_TYPE=Debug", "-DCMAKE_MAKE_PROGRAM=" + ninja], "incremental-configure.log")
    headers = list(incremental.rglob("dependency.h"))
    assert len(headers) == 1
    headers[0].write_text("#define QCAT_DEPENDENCY_VALUE 1\n", encoding="utf-8")
    run([cmake, "--build", incremental], "incremental-build-1.log")
    first = run([incremental / "msvc-incremental.exe"], "incremental-value-1.log", capture=True)
    headers = list(incremental.rglob("dependency.h"))
    assert len(headers) == 1
    headers[0].write_text("#define QCAT_DEPENDENCY_VALUE 2\n", encoding="utf-8")
    run([cmake, "--build", incremental], "incremental-build-2.log")
    second = run([incremental / "msvc-incremental.exe"], "incremental-value-2.log", capture=True)
    assert (first, second) == ("1", "2"), (first, second)
    report["incremental"] = "changed header rebuilt correctly (1 -> 2)"
    (output / "report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report, indent=2), flush=True)

if __name__ == "__main__":
    main()
