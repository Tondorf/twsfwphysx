#include "twsfwphysx/twsfwphysx.h"

#include <string.h>

#ifndef TWSFWPHYSX_VERSION
#  error \
      "TWSFWPHYSX_VERSION is not defined. Ensure the compile definition is passed correctly from CMake."
#endif

int main(const int argc, const char* argv[])
{
  (void)argc;
  (void)argv;

  const char* actual = twsfwphysx_version();
  const char* exptected = TWSFWPHYSX_VERSION;

  return strcmp(actual, exptected) == 0 ? 0 : 1;
}
