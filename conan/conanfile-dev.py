from conan import ConanFile
from conan.tools.files import copy


class ConanPackage(ConanFile):
    """Various helpful utilities - header-only source. From a collection of low-overhead libraries."""
    name = "utils-micro-dev"
    license = "MPL"
    homepage = "TODO"
    description = "Various helpful utilities - header-only source. From a collection of low-overhead libraries."
    settings = "os", "arch", "compiler", "build_type"
    package_type = "header-library"
    python_requires = "base-cmake-library/1.0@micro-libraries/default"
    python_requires_extend = "base-cmake-library.BaseCmakeLibrary"

    options = {}
    default_options = {}

    def build_requirements(self):
        self.test_requires("catch2/3.7.0")

    def requirements(self):
        pass

    def build(self):
        # Since this is a header-only package and even its default configuration may be incompatible
        # with the host system, it has to be tested downstream.
        pass

    def package(self):
        copy(self, "*", self.source_folder / "include" / "dev", self.package_folder / "include")

    def package_info(self):
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []
        self.cpp_info.set_property("cmake_target_name", "utils-micro::utils-micro-dev")
        self.cpp_info.set_property("cmake_file_name", "utils-micro-dev")

    def package_id(self):
        self.info.clear()
