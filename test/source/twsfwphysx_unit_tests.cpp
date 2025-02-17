#include "twsfwphysx/twsfwphysx.h"

#include <cassert>
#include <string>

#ifndef TWSFWPHYSX_VERSION
#error \
	"TWSFWPHYSX_VERSION is not defined. Ensure the compile definition is passed correctly from CMake."
#endif

#ifdef NDEBUG
#error "Assertions must be enabled. Compile without defining NDEBUG."
#endif

void test_version()
{
	const std::string actual(twsfwphysx_version());
	const std::string expected(TWSFWPHYSX_VERSION);

	assert(actual == expected);
}

int main(const int argc, const char *argv[])
{
	(void)argc;
	(void)argv;

	test_version();
}
