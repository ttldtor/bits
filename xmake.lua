-- Copyright (c) 2025 ttldtor.
-- SPDX-License-Identifier: BSL-1.0

add_rules("mode.debug", "mode.release")

target("libs")
    set_kind("headeronly")
    add_includedirs("include", {public = true})

option("tests", {default = true, showmenu = true})

if has_config("tests") then
    add_requires("doctest")
    target("bits_tests")
        set_kind("binary")
        add_files("tests/*.cpp")
        add_packages("doctest")
        add_deps("bits")
end
