#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include "twsfwphysx/twsfwphysx.h"
#include "utils.h"

void test_collision1(const int create_buffer)
{
	const struct twsfwphysx_world world = { .restitution = 1.F,
											.agent_radius = .1F,
											.missile_acceleration = 1.F };

	const struct twsfwphysx_agent agent1 = { make_vec(0.F, 1.F, 0.F),
											 make_vec(0.F, 0.F, 1.F),
											 0.F,
											 0.F,
											 5 };
	const struct twsfwphysx_agent agent2 = { make_vec(1.F, 0.F, 0.F),
											 make_vec(0.F, 0.F, 1.F),
											 1.F,
											 1.F,
											 5 };
	struct twsfwphysx_agents agents = twsfwphysx_create_agents(2);
	twsfwphysx_set_agent(&agents, agent1, 0);
	twsfwphysx_set_agent(&agents, agent2, 1);

	struct twsfwphysx_missiles missiles = twsfwphysx_new_missile_batch();

	struct twsfwphysx_simulation_buffer *buffer =
		create_buffer == 1 ? twsfwphysx_create_simulation_buffer() : NULL;

	twsfwphysx_simulate(&agents, &missiles, &world, 2.F, 100, buffer);

	assert_vec_eq(agents.agents[0].u, 0.F, 0.F, 1.F);
	assert_vec_eq(agents.agents[1].u, 0.F, 0.F, 1.F);

	assert(agents.agents[0].v >= 0.F);
	assert(agents.agents[0].v < 1.F);

	assert(agents.agents[1].v >= 0.F);
	assert(agents.agents[1].v < 1.F);

	twsfwphysx_delete_missile_batch(&missiles);
	twsfwphysx_delete_agents(&agents);
	twsfwphysx_delete_simulation_buffer(buffer);
}

void test_collision2(const int create_buffer)
{
	const struct twsfwphysx_world world = { .restitution = 1.F,
											.agent_radius = .1F,
											.missile_acceleration = 1.F };

	const struct twsfwphysx_agent agent1 = { make_vec(0.F, 1.F, 0.F),
											 make_vec(0.F, 0.F, 1.F),
											 1.F,
											 1.F,
											 5 };
	const struct twsfwphysx_agent agent2 = { make_vec(0.F, 0.F, 1.F),
											 make_vec(0.F, -1.F, 0.F),
											 1.F,
											 1.F,
											 5 };
	struct twsfwphysx_agents agents = twsfwphysx_create_agents(2);
	twsfwphysx_set_agent(&agents, agent1, 0);
	twsfwphysx_set_agent(&agents, agent2, 1);

	struct twsfwphysx_missiles missiles = twsfwphysx_new_missile_batch();

	struct twsfwphysx_simulation_buffer *buffer =
		create_buffer == 1 ? twsfwphysx_create_simulation_buffer() : NULL;

	twsfwphysx_simulate(&agents, &missiles, &world, 2.F, 100, buffer);

	assert_vec_eq_with_tolerance(agents.agents[0].u, 0.F, -1.F, 0.F, .2F);
	assert_vec_eq_with_tolerance(agents.agents[1].u, 0.F, 0.F, 1.F, .2F);

	assert(agents.agents[0].v >= 0.F);
	assert(agents.agents[0].v < 1.0001F);

	assert(agents.agents[1].v >= 0.F);
	assert(agents.agents[1].v < 1.0001F);

	twsfwphysx_delete_missile_batch(&missiles);
	twsfwphysx_delete_agents(&agents);
	twsfwphysx_delete_simulation_buffer(buffer);
}

void test_restitution(void)
{
	const struct twsfwphysx_world world1 = { .restitution = 1.F,
											 .agent_radius = .1F,
											 .missile_acceleration = 1.F };

	const struct twsfwphysx_world world2 = { .restitution = .5F,
											 .agent_radius = .1F,
											 .missile_acceleration = 1.F };

	const struct twsfwphysx_agent agent1 = { make_vec(1.F, 0.F, 0.F),
											 make_vec(0.F, 0.F, 1.F),
											 1.F,
											 1.F,
											 5 };
	const struct twsfwphysx_agent agent2 = { make_vec(0.F, 1.F, 0.F),
											 make_vec(0.F, 0.F, -1.F),
											 1.F,
											 1.F,
											 5 };

	struct twsfwphysx_agents agents1 = twsfwphysx_create_agents(2);
	twsfwphysx_set_agent(&agents1, agent1, 0);
	twsfwphysx_set_agent(&agents1, agent2, 1);

	struct twsfwphysx_agents agents2 = twsfwphysx_create_agents(2);
	twsfwphysx_set_agent(&agents2, agent1, 0);
	twsfwphysx_set_agent(&agents2, agent2, 1);

	struct twsfwphysx_missiles missiles = twsfwphysx_new_missile_batch();

	twsfwphysx_simulate(&agents1, &missiles, &world1, 2.F, 100, NULL);
	twsfwphysx_simulate(&agents2, &missiles, &world2, 2.F, 100, NULL);

	assert_vec_eq(agents1.agents[0].u, 0.F, 0.F, -1.F);
	assert_vec_eq(agents2.agents[0].u, 0.F, 0.F, -1.F);
	assert_vec_eq(agents1.agents[1].u, 0.F, 0.F, 1.F);
	assert_vec_eq(agents2.agents[1].u, 0.F, 0.F, 1.F);

	assert(agents1.agents[0].v >= 0.F);
	assert(agents1.agents[1].v >= 0.F);
	assert(agents2.agents[0].v >= 0.F);
	assert(agents2.agents[1].v >= 0.F);

	assert(fabsf(agents1.agents[0].v - agents1.agents[1].v) < 1e-6F);
	assert(fabsf(agents2.agents[0].v - agents2.agents[1].v) < 1e-6F);

	const float v_sum1 = agents1.agents[0].v + agents1.agents[1].v;
	const float v_sum2 = agents2.agents[0].v + agents2.agents[1].v;
	assert(v_sum1 > v_sum2);

	twsfwphysx_delete_missile_batch(&missiles);
	twsfwphysx_delete_agents(&agents1);
	twsfwphysx_delete_agents(&agents2);
}

int main(const int argc, const char *argv[])
{
	(void)argc;
	(void)argv;

	test_collision1(0);
	test_collision1(1);
	test_collision2(0);
	test_collision2(1);

	test_restitution();

	return 0;
}
