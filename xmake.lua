set_project("audivis-relay")
set_policy("compatibility.version", "3.0")

set_languages("c++2b")
set_warnings("all") 
add_rules("plugin.compile_commands.autoupdate", {outputdir = "build"})
add_rules("mode.releasedbg")

-- static link msvcrt
set_runtimes("MT")

target("audivis-relay")
    set_kind("binary")
    add_files("src/client/**.cc")