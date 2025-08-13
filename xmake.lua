set_project("audivis-relay")
set_policy("compatibility.version", "3.0")

set_languages("c++2b")
set_warnings("all") 
add_rules("plugin.compile_commands.autoupdate", {outputdir = "build"})
add_rules("mode.releasedbg")

add_requires("gtest", "libnyquist", "libsamplerate", "cpptrace", "uwebsockets")

-- static link msvcrt
set_runtimes("MT")

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
    add_files("src/client/**.cc")
    add_deps("parsec-vusb-api")
    add_packages("libnyquist", "libsamplerate", "cpptrace", "uwebsockets")
