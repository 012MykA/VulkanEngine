#pragma once

#include "VulkanEngine/Core/Base.hpp"

#include <spdlog/spdlog.h>

namespace VE
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
	
} // namespace VE

// Core log macros
#define VE_CORE_TRACE(...) 		::VE::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define VE_CORE_INFO(...) 		::VE::Log::GetCoreLogger()->info(__VA_ARGS__)
#define VE_CORE_WARN(...) 		::VE::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define VE_CORE_ERROR(...) 		::VE::Log::GetCoreLogger()->error(__VA_ARGS__)
#define VE_CORE_CRITICAL(...)	::VE::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define VE_TRACE(...) 			::VE::Log::GetClientLogger()->trace(__VA_ARGS__)
#define VE_INFO(...) 			::VE::Log::GetClientLogger()->info(__VA_ARGS__)
#define VE_WARN(...) 			::VE::Log::GetClientLogger()->warn(__VA_ARGS__)
#define VE_ERROR(...) 			::VE::Log::GetClientLogger()->error(__VA_ARGS__)
#define VE_CRITICAL(...) 		::VE::Log::GetClientLogger()->critical(__VA_ARGS__)
