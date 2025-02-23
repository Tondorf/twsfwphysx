#include <assert.h>
#include <string.h>

#include "twsfwphysx/twsfwphysx.h"

#ifndef TWSFWPHYSX_VERSION
#error \
	"TWSFWPHYSX_VERSION is not defined. Ensure the compile definition is passed correctly from CMake."
#endif

void test_version(void)
{
	const char *actual = twsfwphysx_version();
	const char *expected = TWSFWPHYSX_VERSION;

	assert(strcmp(actual, expected) == 0);
}

int main(const int argc, const char *argv[])
{
	(void)argc;
	(void)argv;

	test_version();

	return 0;
}
