from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMakeToolchain, CMakeDeps


class RMERecipe(ConanFile):
    name = "rme"
    version = "4.1.2"
    description = "Remere's Map Editor - OTAcademy Edition"
    license = "GPL-3.0"
    
    settings = "os", "compiler", "build_type", "arch"
    
    def requirements(self):
        self.requires("wxwidgets/3.2.6")

        self.requires("asio/1.32.0")
        self.requires("nlohmann_json/3.11.3")
        self.requires("fmt/11.1.4")
        self.requires("libarchive/3.7.7")
        self.requires("boost/1.87.0")
        self.requires("zlib/1.3.1")
        self.requires("opengl/system")
        self.requires("glad/0.1.36")
        self.requires("glm/1.0.1")
        self.requires("nanovg/cci.20230623")
        self.requires("spdlog/1.15.0")

    
    def layout(self):
        cmake_layout(self)
    
    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        
        tc = CMakeToolchain(self, generator="Ninja")
        tc.cache_variables["CMAKE_CXX_STANDARD"] = "20"
        tc.cache_variables["CMAKE_CXX_STANDARD_REQUIRED"] = "ON"
        # Ensure Unicode mode on Windows
        tc.preprocessor_definitions["UNICODE"] = ""
        tc.preprocessor_definitions["_UNICODE"] = ""
        tc.generate()
    
    def configure(self):
        # Boost components needed
        self.options["boost/*"].without_python = True
        self.options["boost/*"].without_test = True
        
        # wxWidgets components needed
        self.options["wxwidgets/*"].opengl = True
        self.options["wxwidgets/*"].aui = True
        self.options["wxwidgets/*"].html = True
        self.options["wxwidgets/*"].unicode = True
