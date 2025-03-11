#include <assert.h>
#include <math.h>

#include "twsfwphysx/twsfwphysx.h"
#include "utils.h"

struct twsfwphysx_vec make_vec(const float x, const float y, const float z)
{
    const struct twsfwphysx_vec v = { x, y, z };
    return v;
}

void assert_vec_eq_with_tolerance(
    const struct twsfwphysx_vec v,
    const float x, // NOLINT(bugprone-easily-swappable-parameters)
    const float y, // NOLINT(bugprone-easily-swappable-parameters)
    const float z, // NOLINT(bugprone-easily-swappable-parameters)
    const float abs // NOLINT(bugprone-easily-swappable-parameters)
)
{
    assert(fabsf(v.x - x) < abs);
    assert(fabsf(v.y - y) < abs);
    assert(fabsf(v.z - z) < abs);
}

void assert_vec_eq(
    const struct twsfwphysx_vec v,
    const float x, // NOLINT(bugprone-easily-swappable-parameters)
    const float y, // NOLINT(bugprone-easily-swappable-parameters)
    const float z // NOLINT(bugprone-easily-swappable-parameters)
)
{
    assert_vec_eq_with_tolerance(v, x, y, z, 1e-5F);
}
