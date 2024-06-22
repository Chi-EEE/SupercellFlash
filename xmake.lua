add_rules("mode.debug", "mode.release")
add_repositories("local-repo local_repository")
set_languages("cxx17")

option("header_only", {default = true, showmenu = true, description = "Build as header only library"})

add_requires("supercell_compression", "supercell_core")

target("supercell_flash")
    set_kind("headeronly")
    
    add_packages("supercell_compression", "supercell_core")

    if not has_config("header_only") then
        set_kind("$(kind)")
        add_files("source/**.cpp")
    end
    add_headerfiles("include/(**.h)", "include/(**.hpp)")
    add_includedirs("include")

target("supercell_flash_test")
    set_default(false)
    set_kind("binary")
    add_files("test/**.cpp")

    add_headerfiles("include/(**.h)", "include/(**.hpp)")
    add_includedirs("include")
    
    add_packages("supercell_compression", "supercell_core")
    add_deps("supercell_flash")
