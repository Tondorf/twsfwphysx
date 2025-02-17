/*!
  \mainpage twsfwphysx Documentation

  Welcome to the twsfwphysx library documentation.

  Here you can explain the library’s purpose, usage examples, and any other
  details.

  For version information, see \ref twsfwphysx_version.
*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Returns the version of the library
 */
const char* twsfwphysx_version(void);

#ifdef TWSFWPHYSX_IMPLEMENTATION

inline const char* twsfwphysx_version(void)
{
  return "0.1.0";
}

#endif

#ifdef __cplusplus
}  // extern "C"
#endif
