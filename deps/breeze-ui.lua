package("breeze-glfw")
    set_base("glfw")
    set_urls("https://github.com/breeze-shell/glfw.git")
    add_versions("latest", "master")

package("breeze-ui")
    add_urls("https://github.com/std-microblock/breeze-ui.git")
    add_versions("2025.08.15", "f0afe3e1ebdd6fcef9c5cbf916ce67ec5bf43a75")
    add_deps("breeze-glfw", "nanovg", "glad", "nanosvg")
    add_configs("shared", {description = "Build shared library.", default = false, type = "boolean", readonly = true})

    if is_plat("windows") then
        add_syslinks("dwmapi")
    end

    on_install("windows", function (package)
        import("package.tools.xmake").install(package, {}, {target = "breeze_ui"})
    end)
