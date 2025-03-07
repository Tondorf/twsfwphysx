#include <assert.h>
#include <math.h>

#include "twsfwphysx/twsfwphysx.h"

void test_agent_rotate(void)
{
	const struct twsfwphysx_vec r = { 1.F, 0.F, 0.F };
	const struct twsfwphysx_vec u = { 0.F, 0.F, 1.F };

	struct twsfwphysx_agent agent = { r, u, 10.F, 7.F, 3 };

	const float angle = .3F;

	twsfwphysx_rotate_agent(&agent, angle);
	const float cos_angle1 =
		(agent.r.x * r.x) + (agent.r.y * r.y) + (agent.r.z * r.z);

	twsfwphysx_rotate_agent(&agent, angle);
	const float cos_angle2 =
		(agent.r.x * r.x) + (agent.r.y * r.y) + (agent.r.z * r.z);

	twsfwphysx_rotate_agent(&agent, -angle);
	const float cos_angle3 =
		(agent.r.x * r.x) + (agent.r.y * r.y) + (agent.r.z * r.z);

	twsfwphysx_rotate_agent(&agent, 0.F);
	const float cos_angle4 =
		(agent.r.x * r.x) + (agent.r.y * r.y) + (agent.r.z * r.z);

	assert(fabsf(cos_angle1 - cosf(angle)) < 1e-6F);
	assert(fabsf(cos_angle2 - cosf(2.F * angle)) < 1e-6F);
	assert(fabsf(cos_angle3 - cos_angle1) < 1e-6F);
	assert(fabsf(cos_angle4 - cos_angle1) < 1e-6F);
}

int main(const int argc, const char *argv[])
{
	(void)argc;
	(void)argv;

	test_agent_rotate();

	return 0;
}
