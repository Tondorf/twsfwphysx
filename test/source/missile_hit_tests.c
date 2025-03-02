#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "utils.h"
#include "twsfwphysx/twsfwphysx.h"

void test_missile_hit_single_agent(const int create_buffer)
{
	const struct twsfwphysx_world world = { .restitution = 1.F,
											.agent_radius = .1F,
											.missile_acceleration = 1.F };

	const struct twsfwphysx_agent agent = { make_vec(0.F, 0.F, 1.F),
											make_vec(-1.F, 0.F, 0.F),
											0.F,
											0.F,
											5 };
	struct twsfwphysx_agents agents = twsfwphysx_create_agents(2);
	twsfwphysx_set_agent(&agents, agent, 0);

	const struct twsfwphysx_missile m = { make_vec(0.F, 1.F, 0.F),
										  make_vec(1.F, 0.F, 0.F),
										  1.F,
										  42 };
	struct twsfwphysx_missiles missiles = twsfwphysx_new_missile_batch();
	twsfwphysx_add_missile(&missiles, m);

	struct twsfwphysx_simulation_buffer *buffer =
		create_buffer == 1 ? twsfwphysx_create_simulation_buffer() : NULL;

	twsfwphysx_simulate(&agents, &missiles, &world, 2.F, 100, buffer);
	assert(missiles.size == 0);
	assert(fabsf(agents.agents[0].hp - 4.F) < 1e-6F);

	twsfwphysx_delete_missile_batch(&missiles);
	twsfwphysx_delete_agents(&agents);
	twsfwphysx_delete_simulation_buffer(buffer);
}

void test_missile_hit_two_agents(const int create_buffer)
{
	const struct twsfwphysx_world world = { .restitution = 1.F,
											.agent_radius = .1F,
											.missile_acceleration = 1.F };

	const struct twsfwphysx_agent agent1 = { make_vec(1.F, 0.F, 0.F),
											 make_vec(0.F, 0.F, 1.F),
											 0.F,
											 0.F,
											 5 };
	const struct twsfwphysx_agent agent2 = { make_vec(0.F, 0.F, 1.F),
											 make_vec(-1.F, 0.F, 0.F),
											 0.F,
											 0.F,
											 5 };
	struct twsfwphysx_agents agents = twsfwphysx_create_agents(2);
	twsfwphysx_set_agent(&agents, agent1, 0);
	twsfwphysx_set_agent(&agents, agent2, 1);

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

	struct twsfwphysx_simulation_buffer *buffer =
		create_buffer == 1 ? twsfwphysx_create_simulation_buffer() : NULL;

	twsfwphysx_simulate(&agents, &missiles, &world, 2.F, 100, buffer);
	assert(missiles.size == 1);
	assert(missiles.missiles[0].payload == 42);
	assert(fabsf(agents.agents[0].hp - 5.F) < 1e-6F);
	assert(fabsf(agents.agents[1].hp - 4.F) < 1e-6F);

	twsfwphysx_simulate(&agents, &missiles, &world, 2.F, 100, buffer);
	assert(missiles.size == 0);
	assert(fabsf(agents.agents[0].hp - 2) < 1e-6F);
	assert(fabsf(agents.agents[1].hp - 4) < 1e-6F);

	twsfwphysx_delete_missile_batch(&missiles);
	twsfwphysx_delete_agents(&agents);
	twsfwphysx_delete_simulation_buffer(buffer);
}

int main(const int argc, const char *argv[])
{
	(void)argc;
	(void)argv;

	test_missile_hit_single_agent(0);
	test_missile_hit_single_agent(1);
	test_missile_hit_two_agents(0);
	test_missile_hit_two_agents(1);

	return 0;
}
