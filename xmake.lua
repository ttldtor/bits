-- Copyright (c) 2025 ttldtor.
-- SPDX-License-Identifier: BSL-1.0

add_rules("mode.debug", "mode.release")

target("libs")
    set_kind("headeronly")
    add_includedirs("include", {public = true})
