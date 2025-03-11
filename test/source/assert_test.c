#include <assert.h>

#ifdef NDEBUG
#error "Assertions must be enabled. Compile without defining NDEBUG."
#endif

int main(const int argc, const char *argv[])
{
    (void)argc;
    (void)argv;

    int ret = -1;
    assert(++ret == 0); // NOLINT(bugprone-assert-side-effect)
    return ret;
}
