#include "Log.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace VE
{
	Ref<spdlog::logger> Log::m_CoreLogger;
	Ref<spdlog::logger> Log::m_ClientLogger;

	void Log::Init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");
		m_CoreLogger = spdlog::stdout_color_mt("VulkanEngine");
		m_CoreLogger->set_level(spdlog::level::trace);

		m_ClientLogger = spdlog::stdout_color_mt("App");
		m_ClientLogger->set_level(spdlog::level::trace);
	}

} // namespace VE
