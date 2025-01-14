add_rules("mode.debug", "mode.release")
add_repositories("local-repo local_repository")
set_languages("cxx17")

add_requires("supercell_compression", "supercell_core")

target("supercell_flash")
    set_kind("$(kind)")
    
    add_packages("supercell_compression", "supercell_core")

    add_files("source/**.cpp")
    add_headerfiles("include/(**.h)", "include/(**.hpp)")
    add_includedirs("include")
    
    if is_plat("windows") and is_kind("shared") then
        add_rules("utils.symbols.export_all", {export_classes = true})
    end

target("supercell_flash_test")
    set_default(false)
    set_kind("binary")
    add_files("test/**.cpp")

    add_headerfiles("include/(**.h)", "include/(**.hpp)")
    add_includedirs("include")
    
    add_packages("supercell_compression", "supercell_core")
    add_deps("supercell_flash")
