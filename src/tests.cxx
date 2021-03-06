// Project Config
#include "config.hh"

#define V4D_LOGGER_PREFIX " [tests] "

// V4D Core Header
#include <v4d.h>

// Other tests
#include "v4d/core/tests.cxx"

namespace MyProject {
	int MyUnitTest1() {
		// return 0 for succes, anything else for failure
		return 0;
	}
}

START_UNIT_TESTS

	RUN_UNIT_TESTS( V4D_CORE )
	RUN_UNIT_TESTS( MyProject::MyUnitTest1 )

END_UNIT_TESTS
