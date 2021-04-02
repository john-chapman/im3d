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

workspace "im3d_opengl33"
	location(_ACTION)
	configurations { "Debug", "Release" }
	if os.host() == "windows" then
		platforms { "Win32", "Win64" }
	elseif os.host() == "linux" then
		platforms { "Linux64" }
	end
	cppdialect "C++11"
	staticruntime "On"

	filter { "platforms:*32" }
	  architecture "x86"
	filter { "platforms:*64" }
	  architecture "x64"

	filter { "platforms:Win32 or platforms:Win64" }
	  system "windows"
	  links { "opengl32", "gdi32" }

	filter { "platforms:Linux64" }
	  system "linux"
	  links { "GL", "dl", "pthread", "X11" }

	filter { "platforms:*" }
	  defines { "IM3D_OPENGL", "GLEW_STATIC" }

	vpaths({
		["im3d"]   = { IM3D_DIR .. "*.h", IM3D_DIR .. "*.cpp" },
		["imgui"]  = EXAMPLE_COMMON_DIR .. "imgui/**",
		["common"] = { EXAMPLE_COMMON_DIR .. "*.h", EXAMPLE_COMMON_DIR .. "*.cpp" },
		["*"]      = "*.cpp"
		})

	project "im3d_opengl33"
		kind "ConsoleApp"
		language "C++"
		targetdir ""

		defines { "IM3D_OPENGL_VMAJ=3", "IM3D_OPENGL_VMIN=3", "IM3D_OPENGL_VSHADER=330" }

		if os.host() == "linux" then
			includedirs { EXAMPLE_COMMON_DIR .. "glfw/glfw-3.3.3/include/" }
			links { "GLFW" }
		end

		includedirs({
			IM3D_DIR,
			EXAMPLE_COMMON_DIR,
			})
		files({
			IM3D_DIR .. "im3d.cpp",
			IM3D_DIR .. "im3d*.h",
			EXAMPLE_COMMON_DIR .. "**.h",
			EXAMPLE_COMMON_DIR .. "**.hpp",
			EXAMPLE_COMMON_DIR .. "GL/glew.c",
			EXAMPLE_COMMON_DIR .. "**.cpp",
			"*.cpp"
			})

		if os.host() ~= "linux" then
			excludes({
				EXAMPLE_COMMON_DIR .."glfw/**"
			})
		end

if os.host() == "linux" then
	include(EXAMPLE_COMMON_DIR .. "glfw/glfw.lua")
end
