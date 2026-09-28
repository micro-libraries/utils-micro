import os
import re
from pathlib import Path
from typing import Any

from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.env import Environment
from conan.tools.files import load, update_conandata
from conan.tools.scm import Git


def cmake_bool(value: Any) -> str:
    return "ON" if value else "OFF"


def strtobool(value: str) -> bool:
    return value.lower() in ["y", "yes", "t", "true", "1"]


class BaseCmakeLibrary:
    def init(self) -> None:
        self.no_copy_source = True

    def set_version(self) -> None:
        git = Git(self, self.recipe_folder)
        content = load(self, Path(git.get_repo_root()) / "CMakeLists.txt")
        version = re.search("VERSION (.*) LANGUAGES", content).group(1)
        self.version = version.strip()

    def validate(self):
        check_min_cppstd(self, "17")

    def layout(self) -> None:
        self.folders.root = ".."
        cmake_layout(self)

    def export(self) -> None:
        git = Git(self)
        scm_url, scm_commit = git.get_url_and_commit(repository=True)
        update_conandata(self, {"scm": {"commit": scm_commit, "url": scm_url}})

    def source(self) -> None:
        git = Git(self)
        git.fetch_commit(**self.conan_data["scm"])

    def _propagate_options_to_cmake(self, tc: CMakeToolchain) -> None:
        pass

    def generate(self) -> None:
        tc = CMakeToolchain(self)
        tc.user_presets_path = None
        self._propagate_options_to_cmake(tc)
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    # The RUN_CMAKE_* environment variables are used for selectively disabling parts of the build process
    # to speed up development or when something is unfinished. They shouldn't be used to prepare a release.

    def build(self) -> None:
        cmake = CMake(self)
        cmake.configure()

        if strtobool(os.environ.get("RUN_CMAKE_BUILD", "True")):
            build_tool_args = []
            cpu_count = os.environ.get("CONAN_CPU_COUNT", None)
            if cpu_count:
                build_tool_args.extend(["-j", cpu_count])
            cmake.build(build_tool_args=build_tool_args)

        if strtobool(os.environ.get("RUN_CMAKE_TEST", "True")):
            env = Environment()
            env.define("CTEST_OUTPUT_ON_FAILURE", "ON")
            with env.vars(self).apply():
                cmake.test()

    def package(self) -> None:
        cmake = CMake(self)

        if strtobool(os.environ.get("RUN_CMAKE_INSTALL", "True")):
            cmake.install()


class BaseCmakeLibraryPackage(ConanFile):
    name = "base-cmake-library"
    version = "1.0"
    description = "Base for various Conan packages using CMake. Simplifies building from source."
    package_type = "python-require"
