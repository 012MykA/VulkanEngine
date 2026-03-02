#pragma once

#include "VulkanEngine/Core/Base.hpp"

#include <spdlog/spdlog.h>

namespace ve
{
	class Log
	{
	public:
		static void Init();

		static Ref<spdlog::logger> &GetCoreLogger() { return m_CoreLogger; }
		static Ref<spdlog::logger> &GetClientLogger() { return m_ClientLogger; }

	private:
		static Ref<spdlog::logger> m_CoreLogger;
		static Ref<spdlog::logger> m_ClientLogger;
	};
	
} // namespace ve

// Core log macros
#define VE_CORE_TRACE(...) 		::ve::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define VE_CORE_INFO(...) 		::ve::Log::GetCoreLogger()->info(__VA_ARGS__)
#define VE_CORE_WARN(...) 		::ve::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define VE_CORE_ERROR(...) 		::ve::Log::GetCoreLogger()->error(__VA_ARGS__)
#define VE_CORE_CRITICAL(...)	::ve::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define VE_TRACE(...) 			::ve::Log::GetClientLogger()->trace(__VA_ARGS__)
#define VE_INFO(...) 			::ve::Log::GetClientLogger()->info(__VA_ARGS__)
#define VE_WARN(...) 			::ve::Log::GetClientLogger()->warn(__VA_ARGS__)
#define VE_ERROR(...) 			::ve::Log::GetClientLogger()->error(__VA_ARGS__)
#define VE_CRITICAL(...) 		::ve::Log::GetClientLogger()->critical(__VA_ARGS__)
