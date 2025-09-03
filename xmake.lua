-- Copyright (c) 2025 ttldtor.
-- SPDX-License-Identifier: BSL-1.0

package("bits")
    set_kind("library", {headeronly = true})
    set_homepage("https://github.com/ttldtor/bits")
    set_description("Library for bit manipulation")
    set_license("BSL-1.0")

    add_urls("https://github.com/ttldtor/bits.git")

    on_install(function (package)
        os.cp("include/bits", package:installdir("include"))
    end)
package_end()

add_rules("mode.debug", "mode.release")

target("bits")
    set_kind("headeronly")
    add_includedirs("include", {public = true})
    set_languages("c++20")

option("tests", {default = true, showmenu = true})

if has_config("tests") then
    add_requires("doctest")
    target("bits_tests")
        set_kind("binary")
        add_files("tests/*.cpp")
        set_languages("c++20")
        add_packages("doctest")
        add_deps("bits")
end
