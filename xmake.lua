add_rules("mode.debug", "mode.release")

add_repositories("local-repo local_repository")

set_languages("cxx17")

add_requires("supercell_compression", "supercell_core")

target("supercell_flash")
    set_kind("$(kind)")
    
    add_packages("supercell_compression", "supercell_core")

    add_files("source/**.cpp")
    add_headerfiles("include/(**.h)")
    add_includedirs("include")
