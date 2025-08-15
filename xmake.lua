set_project("audivis-relay")
set_policy("compatibility.version", "3.0")

set_languages("c++2b")
set_warnings("all") 
add_rules("plugin.compile_commands.autoupdate", {outputdir = "build"})
add_rules("mode.releasedbg")

includes("deps/libdatachannel.lua")
includes("deps/breeze-ui.lua")

add_requires("gtest", "libnyquist", "libsamplerate", "cpptrace", "cpp-httplib", "nlohmann_json", "libdatachannel", "breeze-ui", "qr-code-generator-cpp", "yy-thunks")

set_policy("build.optimization.lto", true)

target("parsec-vusb-api")
    set_kind("static")
    add_files("src/parsec-vusb-api/**.cc")
    add_headerfiles("src/parsec-vusb-api/**.h")
    add_syslinks("setupapi", "user32", "kernel32")
    add_includedirs("src/parsec-vusb-api", {public = true})

target("parsec-vusb-tests")
    set_kind("binary")
    add_files("src/parsec-vusb-tests/**.cc")
    add_deps("parsec-vusb-api")
    add_packages("gtest")
    set_default(false)

target("audivis-relay")
    set_kind("binary")
    add_files("src/client/**.cc", "src/client/app.manifest")
    set_encodings("utf-8")
    add_deps("parsec-vusb-api")
    add_packages("cpptrace", "libdatachannel", "cpp-httplib", "nlohmann_json", "breeze-ui", "qr-code-generator-cpp", "yy-thunks")
