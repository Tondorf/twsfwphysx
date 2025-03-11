#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

#include "utils.h"
#include "twsfwphysx/twsfwphysx.h"

float length(struct twsfwphysx_vec v)
{
    return sqrtf((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
}

void test_propagation_with_friction(void)
{
    const struct twsfwphysx_world world = { .restitution = 1.F,
                                            .agent_radius = .1F,
                                            .missile_acceleration = 1.F };

    const float v = 1.F;
    const float a = 2.F;
    const struct twsfwphysx_agent agent = { make_vec(1.F, 0.F, 0.F),
                                            make_vec(0.F, 0.F, 1.F),
                                            v,
                                            a,
                                            5 };
    struct twsfwphysx_agents agents = twsfwphysx_create_agents(1);

    struct twsfwphysx_missiles missiles = twsfwphysx_new_missile_batch();

    const float t = .1F;
    const int32_t n_steps[] = { 1, 2, 10 };
    for (int i = 0; i < 3; i++) {
        twsfwphysx_set_agent(&agents, agent, 0);
        twsfwphysx_simulate(&agents, &missiles, &world, t, n_steps[i], NULL);

        assert_vec_eq(agents.agents[0].u, 0.F, 0.F, 1.F);

        const float v_exp = (v * expf(-t)) - (a * expm1f(-t));
        assert(fabsf(agents.agents[0].v - v_exp) < 1e-5F);

        const float s_exp = (a * t) + ((a - v) * expm1f(-t));
        assert(fabsf(length(agents.agents[0].r) - 1.F) < 1e-5F);
        assert(fabsf(agents.agents[0].r.x - cosf(s_exp)) < 1e-5F);
        assert(fabsf(agents.agents[0].r.z) < 1e-5F);
    }

    twsfwphysx_delete_missile_batch(&missiles);
    twsfwphysx_delete_agents(&agents);
}

int main(const int argc, const char *argv[])
{
    (void)argc;
    (void)argv;

    test_propagation_with_friction();

    return 0;
}
