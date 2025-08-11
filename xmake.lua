set_project("audivis-relay")
set_policy("compatibility.version", "3.0")

set_languages("c++2b")
set_warnings("all") 
add_rules("plugin.compile_commands.autoupdate", {outputdir = "build"})
add_rules("mode.releasedbg")

target("audivis-relay-driver")
    add_rules("wdk.driver", "wdk.env.kmdf")
    add_files("src/driver/**.cc", "src/driver/**.cpp")
    add_links("portcls", "ksguid", "uuid", "stdunk", "libcntpr")
    add_cxxflags("/DNTDDI_VERSION=0x0A000008")
    add_defines("_NEW_DELETE_OPERATORS_", "_USE_WAVERT_")
    set_encodings("utf-8")

target("audivis-relay")
    set_kind("binary")
    add_files("src/client/**.cc")