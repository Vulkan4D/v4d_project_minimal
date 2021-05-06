#pragma once

// Logger
#if (defined(_DEBUG) && defined(_LINUX)) || defined(_V4D_TESTS)
	#define V4D_LOGGER_INSTANCE v4d::io::Logger::ConsoleInstance()
#else
	#define V4D_LOGGER_INSTANCE v4d::io::Logger::FileInstance("output.log")
#endif
