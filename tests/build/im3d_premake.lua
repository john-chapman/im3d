local IM3D_DIR  = "../../"
local TESTS_DIR = "../../tests/"

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

workspace "im3d"
	location(_ACTION)
	configurations { "Debug", "Release" }
	platforms { "Win32", "Win64" }
	flags { "C++11", "StaticRuntime" }

	filter { "platforms:Win32 or Win64" }
		system "windows"

	 -- \todo select graphics lib?
		defines { "IM3D_GL", "GLEW_STATIC" }
		links { "opengl32" }

	filter { "platforms:Win32" }
		architecture "x86"
	filter { "platforms:Win64" }
		architecture "x86_64"
		
	filter {}
	
	vpaths({
		["im3d"]  = { IM3D_DIR .. "*.h", IM3D_DIR .. "*.cpp" },
		["tests"] = TESTS_DIR .. "**",
		})
	
	files({ 
		IM3D_DIR .. "*.h", 
		IM3D_DIR .. "*.cpp",
		})
	
	project "im3d_tests"
		kind "ConsoleApp"
		language "C++"
		targetdir "../bin"
	
		
		includedirs({
			IM3D_DIR,
			TESTS_DIR,
			})
		files({
			TESTS_DIR .. "**.h",
			TESTS_DIR .. "**.hpp",
			TESTS_DIR .. "**.c",
			TESTS_DIR .. "**.cpp",
			})
