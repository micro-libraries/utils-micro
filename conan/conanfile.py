from conan import ConanFile


class ConanPackage(ConanFile):
    """Various helpful utilities - runtime library. From a collection of low-overhead libraries."""
    name = "utils-micro"
    license = "MIT"
    homepage = "TODO"
    description = "Various helpful utilities - runtime library. From a collection of low-overhead libraries."
    settings = "os", "arch", "compiler", "build_type"
    package_type = "library"
    python_requires = "base-cmake-library/1.0"
    python_requires_extend = "base-cmake-library.BaseCmakeLibrary"

    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }

    default_options = {
        "shared": True,
        "fPIC": True,
    }

    def build_requirements(self):
        self.test_requires("catch2/3.7.0")

    def requirements(self):
        pass
        # if self.options.python_bindings:
        #     self.requires("pybind11/2.13.6")

    def package_info(self):
        self.cpp_info.set_property("cmake_target_name", "utils-micro::utils-micro")
        self.cpp_info.set_property("cmake_file_name", "utils-micro")
