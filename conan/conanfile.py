from conans import ConanFile, tools
from conan.tools.cmake import CMake
from conan.tools.files import copy

from os import path
from os import getcwd


class MarkovJuniorConan(ConanFile):
    name = "MarkovJunior"
    license = "MIT"
    author = "FranzPoize"
    url = "https://github.com/Shreadeagle/MarkovJunior.cpp"
    description = "Markov Junior implementation in cpp"
    topics = ("markov", "wave function collapse")
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "build_tests": [True, False],
    }
    default_options = {
        "shared": False,
        "build_tests": False,
    }

    requires = (
        ("pugixml/1.12.1"),
        ("handy/4ecfa5b125@adnn/develop"),
        ("math/72087b9ee3@adnn/develop"),
    )

    build_policy = "missing"
    generators = "CMakeDeps", "CMakeToolchain"

    scm = {
        "type": "git",
        "url": "auto",
        "revision": "auto",
        "submodule": "recursive",
    }
    python_requires="shred_conan_base/0.0.2@adnn/develop"
    python_requires_extend="shred_conan_base.ShredBaseConanFile"


    def _generate_cmake_configfile(self):
        """ Generates a conanuser_config.cmake file which includes the file generated by """
        """ cmake_paths generator, and forward the remaining options to CMake. """
        with open("conanuser_config.cmake", "w") as config:
            config.write("message(STATUS \"Including user generated conan config.\")\n")
            # avoid path.join, on Windows it outputs '\', which is a string escape sequence.
            #config.write("include(\"{}\")\n".format("${CMAKE_CURRENT_LIST_DIR}/conan_paths.cmake"))
            config.write("set({} {})\n".format("BUILD_tests", self.options.build_tests))
            config.write("set(CMAKE_EXPORT_COMPILE_COMMANDS 1)\n".format("BUILD_tests", self.options.build_tests))


    def export_sources(self):
        # The path of the CMakeLists.txt we want to export is one level above
        folder = path.join(self.recipe_folder, "..")
        copy(self, "*.txt", folder, self.export_sources_folder)


    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.configure()
        return cmake


    def configure(self):
        tools.check_min_cppstd(self, "17")


    def generate(self):
           self._generate_cmake_configfile()

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()


    def package(self):
        cmake = self._configure_cmake()
        cmake.install()


    def package_info(self):
        #self.cpp_info.libs = tools.collect_libs(self)
        self.cpp_info.set_property("cmake_find_mode", "none")
        if self.folders.build_folder:
            self.cpp_info.builddirs.append(self.folders.build_folder)
        else:
            self.cpp_info.builddirs.append(path.join('lib', 'cmake'))
