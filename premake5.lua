
newoption
{
    trigger = "graphics",
    value = "OPENGL_VERSION",
    description = "version of OpenGL to build raylib against",
    allowed = {
        { "opengl11", "OpenGL 1.1"},
        { "opengl21", "OpenGL 2.1"},
        { "opengl33", "OpenGL 3.3"},
        { "opengl43", "OpenGL 4.3"}
    },
    default = "opengl33"
}

function string.starts(String,Start)
    return string.sub(String,1,string.len(Start))==Start
end

workspaceName = "GameTranslator"

workspace (workspaceName)
    configurations { "Debug", "Release"}
    platforms { "x64", "x86", "ARM64"}
	
	defaultplatform ("x64")

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

    filter { "platforms:x64" }
        architecture "x86_64"
		
	filter { "platforms:Arm64" }
        architecture "ARM64"

    filter {}

    targetdir "_bin/%{cfg.buildcfg}/"

    startproject(workspaceName)

    cdialect "C99"
    cppdialect "C++11"
    
include("third-party/raylib-master")
include("third-party/zlib-master")

project (workspaceName)
  	kind "ConsoleApp"
    location "build"
    targetdir "bin/%{cfg.buildcfg}"
	
    filter "configurations:Release"
		kind "WindowedApp"
		entrypoint "mainCRTStartup"

	filter "action:vs*"
        debugdir "$(SolutionDir)"
		
	filter {"action:vs*", "configurations:Release"}
			kind "WindowedApp"
			entrypoint "mainCRTStartup"
	filter {}

	files {"src/**.c", "src/**.cpp", "src/**.h", "src/**.hpp"}

	includedirs { "src" }

-- libraries
	includedirs { "third-party"}
	includedirs { "third-party/raygui-master/src"}
	includedirs { "third-party/vita-headers-master/include"}
	
	link_raylib();
	links {"zlib"}
