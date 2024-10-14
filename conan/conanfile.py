from conans import ConanFile, tools
from conan.tools.cmake import CMake
from conan.tools.files import copy

from os import path
from os import getcwd


class MarkovJuniorConan(ConanFile):
    name = "MarkovJunior.cpp"
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
        ("pugixml/1.14"), #MIT license
        ("handy/15a1bb8eaa@adnn/develop"),
        ("math/d5fa5b3887@adnn/develop"),
        ("graphics/3a7f5997a1@adnn/develop"),
    )

    build_policy = "missing"
    generators = "CMakeDeps", "CMakeToolchain"

    scm = {
        "type": "git",
        "url": "auto",
        "revision": "auto",
        "submodule": "recursive",
    }
    python_requires="shred_conan_base/0.0.5@adnn/stable"
    python_requires_extend="shred_conan_base.ShredBaseConanFile"
