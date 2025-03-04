#include <assert.h>
#include <math.h>

#include "twsfwphysx/twsfwphysx.h"

void test_launch_missile(void)
{
	const struct twsfwphysx_vec r = { 1.F, 0.F, 0.F };
	const struct twsfwphysx_vec u = { 0.F, 0.F, 1.F };

	struct twsfwphysx_world world = { 1.F, .1F, 2.F };
	struct twsfwphysx_agent agent = { r, u, 10.F, 7.F, 3 };
	struct twsfwphysx_missile missile =
		twsfwphysx_launch_missile(&agent, &world);

	const float cos_distance = (agent.r.x * missile.r.x) +
							   (agent.r.y * missile.r.y) +
							   (agent.r.z * missile.r.z);

	const float cos_min_distance = cosf(world.agent_radius);

	assert(cos_distance < cos_min_distance);
	assert(fabsf(cos_distance - cosf(world.agent_radius)) < 1e-5F);

	assert(fabsf(agent.r.z - missile.r.z) < 1e-6F);

	assert(fabsf(agent.u.x - missile.u.x) < 1e-6F);
	assert(fabsf(agent.u.y - missile.u.y) < 1e-6F);
	assert(fabsf(agent.u.z - missile.u.z) < 1e-6F);
}

int main(const int argc, const char *argv[])
{
	(void)argc;
	(void)argv;

	test_launch_missile();

	return 0;
}
