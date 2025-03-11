#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>

#include "twsfwphysx/twsfwphysx.h"
#include "utils.h"

void test_no_agents_no_missiles(const int create_buffer)
{
    const struct twsfwphysx_world world = { .restitution = 1.F,
                                            .agent_radius = .1F,
                                            .missile_acceleration = 1.F };
    struct twsfwphysx_agents agents = twsfwphysx_create_agents(0);
    struct twsfwphysx_missiles missiles = twsfwphysx_new_missile_batch();
    struct twsfwphysx_simulation_buffer *buffer =
        create_buffer == 1 ? twsfwphysx_create_simulation_buffer() : NULL;

    twsfwphysx_simulate(&agents, &missiles, &world, 10.F, 1000, buffer);

    assert(agents.size == 0);
    assert(missiles.size == 0);

    twsfwphysx_delete_agents(&agents);
    twsfwphysx_delete_missile_batch(&missiles);
    twsfwphysx_delete_simulation_buffer(buffer);
}

void test_no_agents_two_missiles(const int create_buffer)
{
    const struct twsfwphysx_world world = { .restitution = 1.F,
                                            .agent_radius = .1F,
                                            .missile_acceleration = 1.F };
    struct twsfwphysx_agents agents = twsfwphysx_create_agents(0);
    struct twsfwphysx_simulation_buffer *buffer =
        create_buffer == 1 ? twsfwphysx_create_simulation_buffer() : NULL;

    const struct twsfwphysx_missile m1 = { make_vec(1.F, 0.F, 0.F),
                                           make_vec(0.F, 0.F, 1.F),
                                           1.F,
                                           42 };
    const struct twsfwphysx_missile m2 = { make_vec(-1.F, 0.F, 0.F),
                                           make_vec(0.F, 0.F, -1.F),
                                           1.F,
                                           1337 };

    const float t = 10.F;
    const int32_t n_steps[] = { 1, 2, 100 };
    for (int i = 0; i < 3; i++) {
        struct twsfwphysx_missiles missiles = twsfwphysx_new_missile_batch();
        twsfwphysx_add_missile(&missiles, m1);
        twsfwphysx_add_missile(&missiles, m2);

        twsfwphysx_simulate(&agents, &missiles, &world, t, n_steps[i], buffer);

        assert(agents.size == 0);
        assert(missiles.size == 2);

        assert_vec_eq(missiles.missiles[0].r, cosf(t), sinf(t), 0.F);
        assert_vec_eq(missiles.missiles[0].u, 0.F, 0.F, 1.F);

        assert_vec_eq(missiles.missiles[1].r, -cosf(t), sinf(t), 0.F);
        assert_vec_eq(missiles.missiles[1].u, 0.F, 0.F, -1.F);

        twsfwphysx_delete_missile_batch(&missiles);
    }

    twsfwphysx_delete_agents(&agents);
    twsfwphysx_delete_simulation_buffer(buffer);
}

int main(const int argc, const char *argv[])
{
    (void)argc;
    (void)argv;

    test_no_agents_no_missiles(0);
    test_no_agents_no_missiles(1);
    test_no_agents_two_missiles(0);
    test_no_agents_two_missiles(1);

    return 0;
}
