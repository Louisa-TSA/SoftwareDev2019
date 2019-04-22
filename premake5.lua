
is_64_bit = true

linux, windows = 1, 2

if package.config:sub(1,1) == '\\' then
    linux = nil
else
    windows = nil
end

workspace "TSA"
    language "C++"
    cppdialect "C++17"

    location "build"

    if is_64_bit then
        architecture "x86_64"
    else
        architecture "x86"
    end

    configurations { "Debug", "Release" }

    filter { "configurations:Debug" }
        symbols "On"
        optimize "Off"

    filter { "configurations:Release" }
        symbols "Off"
        optimize "Speed"
        linkoptions  { "-flto" }
        
    filter { }

    targetdir ("build/bin/")

    objdir ("build/obj/")

project "imgui"
    kind "StaticLib"

    files {
        "lib/imgui/imgui_draw.cpp",
        "lib/imgui/imgui_widgets.cpp",
        "lib/imgui/imgui.cpp"
    }

    filter "configurations:Debug"
        defines { "DEBUG" }

    filter "configurations:Release"
        defines { "NDEBUG" }

    filter {}

project "glad"
    language "C"
    kind "StaticLib"

    files {
        "lib/glad/src/glad.c",
    }

    includedirs "lib/glad/include"

    filter "configurations:Debug"
        defines { "DEBUG" }

    filter "configurations:Release"
        defines { "NDEBUG" }

    filter {}


project "braille"
    kind "ConsoleApp"

    includedirs "src"
    includedirs "lib/glad/include"
    includedirs "lib/imgui"

    links { "imgui", "glad", "glfw" }
    
    if windows then
        links { "OpenGL32" }
    else
        links { "GL", "dl" }
    end

    files {
        "src/main.cpp",
    }

    if is_64_bit then
        defines { "TSA_64_BIT" }
    else 
        defines { "TSA_32_BIT" }
    end

    filter "configurations:Debug"
        defines { "DEBUG" }

    filter "configurations:Release"
        defines { "NDEBUG" }

    filter {}