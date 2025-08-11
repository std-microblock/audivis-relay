set_project("audivis-relay")
set_policy("compatibility.version", "3.0")

set_languages("c++2b")
set_warnings("all") 
add_rules("plugin.compile_commands.autoupdate", {outputdir = "build"})
add_rules("mode.releasedbg")

target("audivis-relay-driver")
    add_rules("wdk.driver")
    add_files("src/driver/**.cc")

target("audivis-relay")
    set_kind("binary")
    add_files("src/client/**.cc")