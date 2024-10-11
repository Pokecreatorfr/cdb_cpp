add_rules("mode.debug", "mode.release")

set_languages("c++17")
set_optimize("fastest")

add_requires("ffmpeg")
add_requires("libsdl")
add_requires("libsdl_image")

target("Codabox_Cpp")
    set_kind("binary")
    add_includedirs("include")
    add_files("src/*.cpp")
    add_packages("libsdl" , "ffmpeg", "libsdl_image")
