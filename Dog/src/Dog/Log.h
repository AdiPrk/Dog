#pragma once

#include <memory>

#include "Core.h"
#include "spdlog/spdlog.h"

namespace Dog {
	class DOG_API Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

// Core log macros
#define DOG_CORE_TRACE(...)    ::Dog::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define DOG_CORE_INFO(...)     ::Dog::Log::GetCoreLogger()->info(__VA_ARGS__)
#define DOG_CORE_WARN(...)     ::Dog::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define DOG_CORE_ERROR(...)    ::Dog::Log::GetCoreLogger()->error(__VA_ARGS__)
#define DOG_CORE_CRITICAL(...) ::Dog::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define DOG_TRACE(...)         ::Dog::Log::GetClientLogger()->trace(__VA_ARGS__)
#define DOG_INFO(...)          ::Dog::Log::GetClientLogger()->info(__VA_ARGS__)
#define DOG_WARN(...)          ::Dog::Log::GetClientLogger()->warn(__VA_ARGS__)
#define DOG_ERROR(...)         ::Dog::Log::GetClientLogger()->error(__VA_ARGS__)
#define DOG_CRITICAL(...)      ::Dog::Log::GetClientLogger()->critical(__VA_ARGS__)