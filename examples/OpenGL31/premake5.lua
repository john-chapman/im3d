local IM3D_DIR  = "../../"
local EXAMPLE_COMMON_DIR = "../common/"

filter { "configurations:debug" }
	defines { "IM3D_DEBUG" }
	targetsuffix "_debug"
	symbols "On"
	optimize "Off"
	
filter { "configurations:release" }
	symbols "On"
	optimize "Full"

filter { "action:vs*" }
	defines { "_CRT_SECURE_NO_WARNINGS", "_SCL_SECURE_NO_WARNINGS" }
	characterset "MBCS" -- force Win32 API to use *A variants (i.e. can pass char* for strings)

workspace "im3d_opengl31"
	location(_ACTION)
	configurations { "Debug", "Release" }
	platforms { "Win32", "Win64" }
	cppdialect "C++11"
	staticruntime "On"
	
	filter { "platforms:Win32 or platforms:Win64" }
		system "windows"

	 -- \todo select graphics lib?
		defines { "IM3D_OPENGL", "GLEW_STATIC" }
		links { "opengl32", "gdi32" }

	filter { "platforms:Win32" }
		architecture "x86"
	filter { "platforms:Win64" }
		architecture "x86_64"
		
	filter {}
	
	vpaths({
		["im3d"]   = { IM3D_DIR .. "*.h", IM3D_DIR .. "*.cpp" },
		["imgui"]  = EXAMPLE_COMMON_DIR .. "imgui/**",
		["common"] = { EXAMPLE_COMMON_DIR .. "*.h", EXAMPLE_COMMON_DIR .. "*.cpp" },
		["*"]      = "*.cpp"
		})
	
	files({ 
		IM3D_DIR .. "*.h", 
		IM3D_DIR .. "*.cpp",
		})
	
	project "im3d_opengl31"
		kind "ConsoleApp"
		language "C++"
		targetdir ""
	
		defines { "IM3D_OPENGL_VMAJ=3", "IM3D_OPENGL_VMIN=1", "IM3D_OPENGL_VSHADER=140" }
		defines { "IM3D_VERTEX_ALIGNMENT=16" } -- can also do this via im3d_config.h
		
		includedirs({
			IM3D_DIR,
			EXAMPLE_COMMON_DIR,
			})
		files({
			EXAMPLE_COMMON_DIR .. "**.h",
			EXAMPLE_COMMON_DIR .. "**.hpp",
			EXAMPLE_COMMON_DIR .. "**.c",
			EXAMPLE_COMMON_DIR .. "**.cpp",
			"*.cpp"
			})
