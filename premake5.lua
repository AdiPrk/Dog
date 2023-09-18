workspace "Dog"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories
IncludeDir = {}
IncludeDir["GLFW"] = "Dog/vendor/GLFW/include"

include "Dog/vendor/GLFW"

project "Dog"
	location "Dog"
	kind "SharedLib"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "dogpch.h"
	pchsource "Dog/src/dogpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}"
	}

	links 
	{ 
		"GLFW",
		"opengl32.lib"
	}

	filter "system:windows"
		cppdialect "C++20"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"DOG_PLATFORM_WINDOWS",
			"DOG_BUILD_DLL"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox")
		}

	filter "configurations:Debug"
		defines "DOG_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "DOG_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "DOG_DIST"
		optimize "On"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Dog/vendor/spdlog/include",
		"Dog/src"
	}

	links
	{
		"Dog"
	}

	filter "system:windows"
		cppdialect "C++20"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"DOG_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "DOG_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "DOG_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "DOG_DIST"
		optimize "On"