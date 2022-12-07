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
        ("handy/9f3abc6665@adnn/develop"),
        ("math/96fbbb3dcc@adnn/develop"),
        ("graphics/5bf5ee7f2c@adnn/develop"),
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
