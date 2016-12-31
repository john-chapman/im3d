local IM3D_DIR  = "../../"
local TESTS_DIR = "../"

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

workspace "im3d_tests"
	location(_ACTION)
	configurations { "Debug", "Release" }
	platforms { "Win32", "Win64" }
	flags { "C++11", "StaticRuntime" }
	filter { "platforms:Win32" }
		system "windows"
		architecture "x86"
	filter { "platforms:Win64" }
		system "windows"
		architecture "x86_64"
	
	project "im3d_math_tests"
		kind "ConsoleApp"
		language "C++"
		targetdir "../bin"
		
		vpaths({
			["*"] = IM3D_DIR .. "*",
			})
		
		files({ 
			IM3D_DIR .. "**.h", 
			IM3D_DIR .. "**.hpp", 
			IM3D_DIR .. "**.c", 
			IM3D_DIR .. "**.cpp",
			})
