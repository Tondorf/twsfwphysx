#include <assert.h>

#include "utils.h"
#include "twsfwphysx/twsfwphysx.h"

void test_missile_batch_reset(void)
{
    const struct twsfwphysx_missile m1 = { make_vec(-1.F, 0.F, 0.F),
                                           make_vec(0.F, 0.F, 1.F),
                                           1.F,
                                           42 };
    const struct twsfwphysx_missile m2 = { make_vec(0.F, 1.F, 0.F),
                                           make_vec(1.F, 0.F, 0.F),
                                           1.F,
                                           1337 };
    struct twsfwphysx_missiles missiles = twsfwphysx_new_missile_batch();
    twsfwphysx_add_missile(&missiles, m1);
    twsfwphysx_add_missile(&missiles, m2);
    assert(missiles.size == 2);

    const int old_capacity = missiles.capacity;
    assert(old_capacity >= missiles.size);

    twsfwphysx_clear_missile_batch(&missiles);
    assert(missiles.size == 0);
    assert(missiles.capacity == old_capacity);

    twsfwphysx_add_missile(&missiles, m2);
    twsfwphysx_add_missile(&missiles, m1);
    assert(missiles.size == 2);
    assert(missiles.capacity == old_capacity);

    assert(missiles.missiles[0].payload == m2.payload);
    assert(missiles.missiles[1].payload == m1.payload);

    twsfwphysx_delete_missile_batch(&missiles);
}

int main(const int argc, const char *argv[])
{
    (void)argc;
    (void)argv;

    test_missile_batch_reset();

    return 0;
}
