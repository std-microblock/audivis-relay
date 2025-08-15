package("breeze-glfw")
    set_base("glfw")
    set_urls("https://github.com/breeze-shell/glfw.git")
    add_versions("latest", "master")

package("breeze-ui")
    add_urls("https://github.com/std-microblock/breeze-ui.git")
    add_versions("20250815.5", "aee2d04523fd27a045cb8da7a4bcc82ce60e5650")
    add_deps("breeze-glfw", "nanovg", "glad", "nanosvg")
    add_configs("shared", {description = "Build shared library.", default = false, type = "boolean", readonly = true})

    if is_plat("windows") then
        add_syslinks("dwmapi", "shcore")
    end

    on_install("windows", function (package)
        import("package.tools.xmake").install(package, {}, {target = "breeze_ui"})
    end)
