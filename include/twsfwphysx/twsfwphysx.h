/*!
  \mainpage twsfwphysx Documentation

  Welcome to the twsfwphysx library documentation.

  twsfwphysx is a lightweight physics engine for twsfw (the 'w' stands for
  WASM), designed for handling agents, missiles and their interactions.

  The entire library is implemented in \ref twsfwphysx.h. Define
  `TWSFWPHYSX_IMPLEMENTATION` in **one** source file before including
  \ref twsfwphysx.h. Without this defintion, \ref twsfwphysx.h is a canonical C
  header file that declares the API but does not provide any implementation.

  Internally, twsfwphysx heavily uses functions from `math.h`: On Linux and
  Darwin systems, ensure that this library is available during
  compilation/linking.

  Find more information about installing and on how to contribute on our
  <a href="https://github.com/Tondorf/twsfwphysx">GitHub page</a>.

  For a detailed documentation of the API see \ref twsfwphysx.h.
*/

/*!
 * \file twsfwphysx.h
 * \brief API of twsfwphysx
 *
 * The twsfwphysx physics engine simulates the movements and interactions of
 * agents and missiles, constrained on the surface of a unit sphere.
 * Movements are subjected to a finite friction that scales linearly with
 * velocity. Missiles use a constant acceleration for propulsion yielding a
 * constant terminal velocity after a short acceleration phase whereas agents
 * can change the magnitude of propulsion between subsequent calls to
 * \ref twsfwphysx_simulate.
 *
 * WOLOG, we set the numerical values of both the friction coefficient and the
 * characteristic time to one. This choice effectively makes the numerical
 * values of the propulsion acceleration and the terminal velocity equal.
 * With this simplification, the equation of the instantaneous velocity update
 * from time \f$t\f$ to \f$t+1\f$ becomes:
 *
 * \f[
 *   v_{t+1} = v_\infty - (v_\infty - v_t) \, \mathrm{e}^{-t \,/\, (1\,\mathrm{s})},
 * \f]
 *
 * where \f$v_\infty\f$ is the terminal velocity (here, referred to as
 * \ref twsfwphysx_agent::a and \ref twsfwphysx_world::missile_acceleration).
 * Agents and missiles move along great-circles; given the rotation axis (see,
 * e.g., \ref twsfwphysx_agent.u), the position on this circle is parametrized
 * with an angle \f$\varphi_t\f$. Its update equation reads:
 * \f[
 *   \varphi_{t+1} = \varphi_0 + v_\infty t - (v_\infty - v_t) \left( \mathrm{e}^{-t \,/\, (1\,\mathrm{s})} - 1 \right).
 * \f]
 *
 * Note the limits for both equations! If \f$t > 3\,\mathrm{s}\f$, agents and missiles
 * travel almost linearly with 95% - 100% terminal velocity.
 *
 * Agents and missiles are parametrized via their position on the sphere,
 * \f$\vec{r} = (x, y, z)^\top\f$, and their angular momentum vector,
 * \f$\vec{L} = \vec{r} \times \vec{v} = v \vec{u}\f$ where \f$\vec{r}\f$ and
 * \f$\vec{u}\f$ are unit vectors; see \ref twsfwphysx_agent (and
 * \ref twsfwphysx_missile) for details.
 * This parametrization makes it very easy to either rotate an object around
 * \f$\vec{u}\f$ (movement along a great-circle) or around \f$\vec{r}\f$
 * (turning), e.g., turning an agent by \f$\vartheta\f$ changes \f$\vec{u}\f$
 * according to
 * \f[
 *   \vec{u} \leftarrow \vec{u} \cos \vartheta \, + (\vec{r} \times \vec{u}) \sin \vartheta \,.
 * \f]
 *
 * Agents have a circular cross-section with finite radius. If the
 * cross-sections of two adjacent agents overlap, they collide elastically.
 * Movements of agents (and missiles) are projected onto the surface of the
 * unit sphere which causes a finite leak of kinetic energy during collision.
 * This effect can be increased by setting \ref twsfwphysx_world.restitution to
 * values `< 1.0`.
 *
 * If missiles intersect with the cross-section of an agent, they detonate which
 * affects the \ref twsfwphysx_agent.hp attribute negatively. When agent and
 * missile meet head-to-head, \ref twsfwphysx_agent.hp is reduced by `1`; if
 * the missile hits the agent's back, \ref twsfwphysx_agent.hp is reduced by
 * `3`.
 *
 * Most of the API (see section below for exceptions) is centered around
 * \ref twsfwphysx_simulate. Start reading the documentation from here and then
 * follow the links to learn how to get the required arguments.
 *
 * Besides \ref twsfwphysx_simulate, there are two helper functions:
 * \ref twsfwphysx_turn_agent and \ref twsfwphysx_version.
 * The former helps to turn (the angular momentum vector of) agents by a
 * given angle. The latter returns the version of the API. Expect breaking
 * changes between **all** `0.*.*` releases and an __almost__ stable ABI for
 * between versions within the same major release `>= 1`.
 *
 * HAVE FUN!
 */

#pragma once

#include <stdint.h>

#ifdef TWSFWPHYSX_IMPLEMENTATION
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Returns the version of the library.
 */
const char *twsfwphysx_version(void);

/**
 * @brief Represents a 3D vector.
 */
struct twsfwphysx_vec {
	float x; ///< x-component of the vector
	float y; ///< y-component of the vector
	float z; ///< z-component of the vector
};

/**
 * @brief Represents an agent with position, momentum, and attributes.
 *
 * The position vector `r` points to a position on the unit sphere and thus has
 * to be normalized to `1`. Similarly, `u` is a normalized vector that
 * represents the rotation axis. Scaled by `v`, this vector becomes the angular
 * momentum vector of the agent: `L = u * v`. `u` and `r` are perpendicular,
 * thus their cross product is the normalized velocity vector.
 *
 * Since the friction coefficient is set to `1s`, the (numerical) value of the
 * acceleration, `a`, now equals the (numerical value of the) terminal velocity.
 * Here, we still refer to it as `a` to avoid confusion with the instantaneous
 * velocity, `v`.
 *
 * **Example**
 * \code{.c}
 * struct twsfwphysx_vec3 r = {0.F, 0.F, 1.F};  // North Pole
 * struct twsfwphysx_angular_momentum u = {1, 0.F, 0.F, v};  // x-axis
 *
 * float v = 2.F;
 * float a = 10.F;
 * float hp = 3.F;
 *
 * // A new agent, rotating around the x-axis, currently moving with `v=2` at
 * // the North Pole, has 3 HPs and propels with `a=10.F`.
 * struct twsfwphysx_agent agent = {r, L, v, a, hp};
 *
 * float vx = v * (u.y * r.z - u.z * r.y)  // x-component of velocity vector
 * float vy = v * (u.z * r.x - u.x * r.z)  // y-component of velocity vector
 * float vz = v * (u.x * r.y - u.y * r.x)  // z-component of velocity vector
 * \endcode
 */
struct twsfwphysx_agent {
	struct twsfwphysx_vec r;
	///< Normalized vector pointing to the position of the agent on the unit sphere.

	struct twsfwphysx_vec u; ///< Rotation axis.

	float v; ///< Magnitude of instantaneous velocity

	float a; ///< Acceleration used for propulsion (terminal velocity)

	float hp; ///< Health points
};

/**
 * @brief Container for managing multiple agents.
 *
 * Create a new container using \ref twsfwphysx_create_agents, fill agents with
 * \ref twsfwphysx_set_agent, and use the container to pass active agents to
 * \ref twsfwphysx_simulate. Remember to eventually delete this container via
 * \ref twsfwphysx_delete_agents if it is no longer needed.
 *
 * Note that agents with negative or zero HPs are ignored during simulation.
 * Therefore, one way to (temporarily) _disable_ agents is to set
 * \ref twsfwphysx_agent.hp to negative values.
 *
 * **Example**
 * \code{.c}
 * // some factory function that generates a set of agents
 * extern struct twsfwphysx_agents make_agents(void);
 *
 * struct twsfwphysx_agents crowd = make_agents()
 * for (int i = 0; i < crowd.size; i++) {
 *     struct twsfwphysx_agent *agent = &crowd.agents[i];
 *     if (i % 2 == 0) {   // kill every second agent
 *	       agent.hp = -1.F;  // to motivate the others
 *     }
 * }
 * \endcode
 */
struct twsfwphysx_agents {
	struct twsfwphysx_agent *agents; ///< Pointer to an array of agents
	int32_t size; ///< Number of agents
};

/**
 * @brief Represents a missile with position and angular momentum.
 *
 * See \ref twsfwphysx_agent for explanations for `r`, `u` and `v`.
 * \ref payload is persistent and can be used to, e.g., associate missiles with
 * agent slots.
 *
 * Use \ref twsfwphysx_launch_missile to create new missiles next to agents and
 * \ref twsfwphysx_add_missile to add them to an existing missile batch.
 */
struct twsfwphysx_missile {
	struct twsfwphysx_vec r; ///< Position
	struct twsfwphysx_vec u; ///< Rotation axis
	float v; ///< Velocity magnitude
	int32_t payload; ///< Payload
};

/**
 * @brief Container for managing multiple missiles.
 *
 * Create a new missile batch with \ref twsfwphysx_new_missile_batch and use
 * this container to pass (active) missiles to \ref twsfwphysx_simulate.
 * Remember to eventually delete this batch via
 * \ref twsfwphysx_delete_missile_batch if it is no longer needed.
 *
 * When missiles detonate during simulation, they will be removed from the batch
 * and the remaining missiles will be reordered. Use `size` to check for the
 * current size of the batch, e.g., before iterating over all missiles:
 * \code{.c}
 * // some factory function that generates missiles
 * extern struct twsfwphysx_missiles make_missiles(void);
 *
 * const struct twsfwphysx_missiles batch = make_missiles();
 * for (int i = 0; i < batch.size; i++) {
 *     struct twsfwphysx_agent missile = batch.missiles[i];
 *     // ...
 * }
 * \endcode
 */
struct twsfwphysx_missiles {
	struct twsfwphysx_missile *missiles; ///< Pointer to an array of missiles
	int32_t size; ///< Number of missiles
	int32_t capacity; ///< **Only for internal usage.**
};

/**
 * @brief Represents the physics world configuration and invariants.
 *
 * Note that the movement of agent is constrained onto the surface of the unit
 * sphere. This will cause a finite loss of kinetic energy during collisions
 * even if `restitution` is set to `1.0`. Choose smaller values to increase this
 * effect.
 */
struct twsfwphysx_world {
	float restitution;
	///< Coefficient of restitution. (See comment above.)

	float agent_radius; ///< Radius of cross-section of agents

	float missile_acceleration;
	///< Acceleration used for missiles propuslion (terminal velocity)
};

/**
 * @struct twsfwphysx_simulation_buffer
 * @brief Opaque data structure used to minimize allocations during simulation.
 *
 * Use \ref twsfwphysx_create_simulation_buffer to create such a buffer and
 * \ref twsfwphysx_delete_simulation_buffer to delete it if no longer needed.
 */
struct twsfwphysx_simulation_buffer;

/**
 * @brief Creates a batch of new agents.
 *
 * Creates a batch of new **uninitialized** agents. Use
 * \ref twsfwphysx_set_agent to initialize up to `size` agents.
 *
 * @param size Size of batch
 * @return Batch of agents
 */
struct twsfwphysx_agents twsfwphysx_create_agents(int32_t size);

/**
 * @brief Deletes all agents in a batch.
 *
 * Deletes a batch of agents that was previously created with
 * \ref twsfwphysx_create_agents.
 *
 * @param agents Batch of agents
 */
void twsfwphysx_delete_agents(struct twsfwphysx_agents *agents);

/**
 * @brief Overrides a slot in the agent batch.
 *
 * Sets `agents->agents[index] = agent`. Note that `index` has to be a valid
 * index, i.e., non-negative and smaller than the batch size that was given
 * to \ref twsfwphysx_create_agents for creating `agents`.
 *
 * See \ref twsfwphysx_agent for details on how to initialize `agent`.
 *
 * @param batch Batch of agents
 * @param agent The new agent
 * @param index Index (`0 <= index < agents.size`)
 */
void twsfwphysx_set_agent(const struct twsfwphysx_agents *batch,
						  struct twsfwphysx_agent agent,
						  int32_t index);

/**
 * @brief Creates a new batch of missiles.
 *
 * Creates a new batch of missiles. Add missiles using
 * \ref twsfwphysx_add_missile and remember to delete the batch via
 * \ref twsfwphysx_delete_missile_batch if no longer needed.
 *
 *
 * @return The missile batch
 */
struct twsfwphysx_missiles twsfwphysx_new_missile_batch(void);

/**
 * @brief Deletes missile batch.
 *
 * Deletes a batch of missiles that was previously created with
 * \ref twsfwphysx_new_missile_batch.
 *
 * @param missiles The missile batch
 */
void twsfwphysx_delete_missile_batch(struct twsfwphysx_missiles *missiles);

/**
 * @brief Adds a new missile to the batch.
 *
 * Adds a new missile to the batch. Adding a missile increases
 * \ref twsfwphysx_missiles.size of `batch` by one.
 *
 * @param missiles The missile batch
 * @param missile New missile
 */
void twsfwphysx_add_missile(struct twsfwphysx_missiles *missiles,
							struct twsfwphysx_missile missile);

/**
 * @brief Creates a missile next to the agent
 *
 * Creates a new missile next to the agent. The angular momentum vector of the
 * missile is set to the same value as the one from the agent and the distance
 * between both is slightly larger than \ref twsfwphysx_world.agent_radius.
 * Use \ref twsfwphysx_add_missile to add this missile to a missile batch.
 *
 * @param agent Agent that launches the missile
 * @param world World invariants
 * @return The new missile
 */
struct twsfwphysx_missile
twsfwphysx_launch_missile(const struct twsfwphysx_agent *agent,
						  const struct twsfwphysx_world *world);

/**
 * @brief Creates a new simulation buffer.
 *
 * This buffer is needed during simulation to save intermediary results. See
 * \ref twsfwphysx_simulate for more details and remember to delete this buffer
 * via \ref twsfwphysx_delete_simulation_buffer if no longer needed.
 *
 * @return A new simulation buffer
 */
struct twsfwphysx_simulation_buffer *twsfwphysx_create_simulation_buffer(void);

/**
 * @brief Deletes the simulation buffer.
 *
 * Deletes the simulation buffer that was previously created using
 * \ref twsfwphysx_create_simulation_buffer.
 *
 * @param buffer The simulation buffer
 */
void twsfwphysx_delete_simulation_buffer(
	struct twsfwphysx_simulation_buffer *buffer);

/**
 * @brief Simulates the movements and interactions of agents and missiles.
 *
 * This function simulates the movement of agents and missiles for a time
 * duration `t` by propagating them for `n_steps` _short(er)_ time steps.
 * For each step, collisions between agents with positive HPs are handled and
 * missiles are detonated, if needed. Increasing `n_steps` will improve the
 * collision/detonation detection but also increase the overall execution time.
 * As a rule of thumb, the propagation distance of agents and missiles during
 * one simulation steps should be smaller than
 * \ref twsfwphysx_world.agent_radius.
 *
 * During simulation, a temporary buffer is needed to store intermediary
 * results. If a \ref twsfwphysx_simulation_buffer is provided via `buffer`,
 * the number of memory allocations can be reduced significantly. (In fact, no
 * allocation is needed if the number of particles does not change between
 * consecutive calls to \ref twsfwphysx_simulate!) Read the documentation of
 * \ref twsfwphysx_simulation_buffer to learn how to get (and how to get rid of)
 * such a buffer! If no such buffer is provided (indicated by setting `buffer`
 * to `NULL`), a buffer will be allocated internally and released again at the
 * end of the simulation run.
 *
 * When missiles detonate, they are removed from `missiles` and the list of
 * remaining missiles is reordered. Note that \ref twsfwphysx_missile.payload
 * still stays persistent and thus can help to identify missiles.
 *
 * @param agents Agents
 * @param missiles Missiles
 * @param world World invariants
 * @param t Simulation time
 * @param n_steps Number of simulation steps.
 * @param buffer Simulation buffer (Set to `NULL` if not needed.)
 */
void twsfwphysx_simulate(struct twsfwphysx_agents *agents,
						 struct twsfwphysx_missiles *missiles,
						 const struct twsfwphysx_world *world,
						 float t,
						 int32_t n_steps,
						 struct twsfwphysx_simulation_buffer *buffer);

/**
 * @brief Changes orientation of agent.
 *
 * This helper function rotates the orientation of an agent by the given
 * angle. Setting `alpha = 0` (or to multiples of two pi) does not change
 * the orientation.
 *
 * @param agent Agent
 * @param angle Rotation angle (in radians).
 */
void twsfwphysx_turn_agent(struct twsfwphysx_agent *agent, float angle);

#ifdef TWSFWPHYSX_IMPLEMENTATION

const char *twsfwphysx_version(void)
{
	return "0.8.0";
}

struct twsfwphysx_agents twsfwphysx_create_agents(const int32_t size)
{
	struct twsfwphysx_agents agents = { NULL, 0 };
	if (size < 1) {
		return agents;
	}

	agents.agents = (struct twsfwphysx_agent *)malloc(
		(uint64_t)size * sizeof(struct twsfwphysx_agent));
	agents.size = size;

	return agents;
}

void twsfwphysx_delete_agents(struct twsfwphysx_agents *agents)
{
	free(agents->agents);
	agents->agents = NULL;
	agents->size = 0;
}

void twsfwphysx_set_agent(const struct twsfwphysx_agents *batch,
						  const struct twsfwphysx_agent agent,
						  const int32_t index)
{
	assert(index >= 0 && index < batch->size);
	assert(batch->agents != NULL);
	batch->agents[index] = agent;
}

struct twsfwphysx_missiles twsfwphysx_new_missile_batch(void)
{
	const struct twsfwphysx_missiles missiles = { NULL, 0, 0 };
	return missiles;
}

void twsfwphysx_delete_missile_batch(struct twsfwphysx_missiles *missiles)
{
	free(missiles->missiles);
	missiles->missiles = NULL;
	missiles->size = 0;
	missiles->capacity = 0;
}

void twsfwphysx_add_missile(struct twsfwphysx_missiles *missiles,
							const struct twsfwphysx_missile missile)
{
	if (missiles->size >= missiles->capacity) {
		missiles->capacity = missiles->capacity > 0 ? 2 * missiles->capacity :
													  1;

		// NOLINTNEXTLINE(bugprone-suspicious-realloc-usage)
		missiles->missiles = (struct twsfwphysx_missile *)realloc(
			missiles->missiles,
			(uint64_t)missiles->capacity * sizeof(struct twsfwphysx_missile));
		assert(missiles->missiles != NULL);
	}

	missiles->missiles[missiles->size++] = missile;
}

static float vec_length(const struct twsfwphysx_vec v)
{
	return sqrtf((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
}

static struct twsfwphysx_vec normalize(struct twsfwphysx_vec v)
{
	const float length = vec_length(v);
	v.x /= length;
	v.y /= length;
	v.z /= length;

	return v;
}

static float dot(const struct twsfwphysx_vec v, const struct twsfwphysx_vec w)
{
	return v.x * w.x + v.y * w.y + v.z * w.z;
}

static struct twsfwphysx_vec cross(const struct twsfwphysx_vec v,
								   const struct twsfwphysx_vec w)
{
	const struct twsfwphysx_vec res = { v.y * w.z - v.z * w.y,
										v.z * w.x - v.x * w.z,
										v.x * w.y - v.y * w.x };
	return res;
}

static void
rotate(struct twsfwphysx_vec *r, struct twsfwphysx_vec u, float angle)
{
	const float sin_angle = sinf(angle);
	const float cos_angle = cosf(angle);

	const struct twsfwphysx_vec w = cross(u, *r);
	r->x = cos_angle * r->x + sin_angle * w.x;
	r->y = cos_angle * r->y + sin_angle * w.y;
	r->z = cos_angle * r->z + sin_angle * w.z;
}

static void
propagate(struct twsfwphysx_vec *r,
		  const struct twsfwphysx_vec u,
		  float *v,
		  const float a, // NOLINT(bugprone-easily-swappable-parameters)
		  const float dt) // NOLINT(bugprone-easily-swappable-parameters)
{
	const float theta = (a * dt) - ((*v - a) * expm1f(-dt));
	rotate(r, u, theta);

	*v = a - ((a - *v) * expf(-dt));
}

static void collide(struct twsfwphysx_agent *p1,
					struct twsfwphysx_agent *p2,
					const float epsilon)
{
	struct twsfwphysx_vec n = { p1->r.x - p2->r.x,
								p1->r.y - p2->r.y,
								p1->r.z - p2->r.z };

	const float length = vec_length(n);
	n.x /= length;
	n.y /= length;
	n.z /= length;

	struct twsfwphysx_vec v1 = cross(p1->u, p1->r);
	v1.x *= p1->v;
	v1.y *= p1->v;
	v1.z *= p1->v;

	struct twsfwphysx_vec v2 = cross(p2->u, p2->r);
	v2.x *= p2->v;
	v2.y *= p2->v;
	v2.z *= p2->v;

	const struct twsfwphysx_vec dv = { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
	const float J = (1.F + epsilon) / 2.F * dot(n, dv);

	v1.x = v1.x - J * n.x;
	v1.y = v1.y - J * n.y;
	v1.z = v1.z - J * n.z;

	v2.x = v2.x + J * n.x;
	v2.y = v2.y + J * n.y;
	v2.z = v2.z + J * n.z;

	p1->v = vec_length(v1);
	p2->v = vec_length(v2);

	if (p1->v > 1e-10F) {
		p1->u = normalize(cross(p1->r, v1));
	}

	if (p2->v > 1e-10F) {
		p2->u = normalize(cross(p2->r, v2));
	}
}

static void fill_distance_buffer(const struct twsfwphysx_agent *agents,
								 float *buffer,
								 int32_t n)
{
	assert(agents != NULL || n < 2);
	assert(buffer != NULL || n < 2);

	int32_t k = 0;
	for (int i = 0; i < n; i++) {
		for (int j = i + 1; j < n; j++) {
			buffer[k++] = dot(agents[i].r, agents[j].r);
		}
	}
}

static void hit(struct twsfwphysx_agent *agent,
				struct twsfwphysx_missiles *missiles,
				const int32_t i)
{
	const struct twsfwphysx_missile missile = missiles->missiles[i];
	const float cos_theta = dot(agent->u, missile.u);
	const float damage = 2.F + cos_theta;

	agent->hp -= damage;

	missiles->size -= 1;
	if (i < missiles->size) {
		missiles->missiles[i] = missiles->missiles[missiles->size];
	}
}

static int32_t nearest_hit(const struct twsfwphysx_agents *agents,
						   const struct twsfwphysx_missile missile,
						   const float threshold)
{
	int32_t i_max = -1;
	float s_max = -2.F; // -1 <= dot(.) <= +1
	for (int32_t i = 0; i < agents->size; i++) {
		if (agents->agents[i].hp > 0.F) {
			const float s = dot(agents->agents[i].r, missile.r);
			if (s > threshold && s > s_max) {
				i_max = i;
				s_max = s;
			}
		}
	}

	return i_max;
}

struct twsfwphysx_simulation_buffer {
	struct twsfwphysx_agent *p;
	float *s1;
	float *s2;
	int32_t capacity;
};

struct twsfwphysx_simulation_buffer *twsfwphysx_create_simulation_buffer(void)
{
	struct twsfwphysx_simulation_buffer *buffer =
		(struct twsfwphysx_simulation_buffer *)malloc(
			sizeof(struct twsfwphysx_simulation_buffer));
	buffer->p = NULL;
	buffer->s1 = NULL;
	buffer->s2 = NULL;
	buffer->capacity = 0;

	return buffer;
}

void twsfwphysx_delete_simulation_buffer(
	struct twsfwphysx_simulation_buffer *buffer)
{
	if (buffer != NULL) {
		free(buffer->p);
		free(buffer->s1);
		free(buffer->s2);
		free(buffer);
	}
}

static struct twsfwphysx_simulation_buffer
update_simulation_buffer(struct twsfwphysx_simulation_buffer buffer,
						 const int32_t n_agents)
{
	assert(buffer.capacity >= 0);

	if (n_agents > buffer.capacity) {
		buffer.capacity = n_agents;

		const uint64_t n = (uint64_t)n_agents;

		buffer.p = (struct twsfwphysx_agent *)
			// NOLINTNEXTLINE(bugprone-suspicious-realloc-usage)
			realloc(buffer.p, n * sizeof(struct twsfwphysx_agent));
		assert(buffer.p != NULL);

		if (n_agents > 1) {
			const uint64_t size = (n * (n - 1)) / 2 * sizeof(float);

			// NOLINTNEXTLINE(bugprone-suspicious-realloc-usage)
			buffer.s1 = (float *)realloc(buffer.s1, size);
			assert(buffer.s1 != NULL);

			// NOLINTNEXTLINE(bugprone-suspicious-realloc-usage)
			buffer.s2 = (float *)realloc(buffer.s2, size);
			assert(buffer.s2 != NULL);
		}
	}

	return buffer;
}

void twsfwphysx_simulate(struct twsfwphysx_agents *agents,
						 struct twsfwphysx_missiles *missiles,
						 const struct twsfwphysx_world *world,
						 const float t,
						 int32_t n_steps,
						 struct twsfwphysx_simulation_buffer *buffer)
{
	struct twsfwphysx_simulation_buffer bffr = { NULL, NULL, NULL, 0 };
	if (buffer == NULL) {
		buffer = &bffr;
	}

	const int32_t n_agents = agents->size;
	*buffer = update_simulation_buffer(*buffer, n_agents);

	// !!! WARNING !!!
	// cos(.) makes small angles large and large angles small!
	// Hence, search for distances *above* `*_threshold` when looking for
	// *close* objects.
	const float missile_agent_threshold = cosf(world->agent_radius);
	const float agent_agent_threshold = cosf(2.F * world->agent_radius);

	struct twsfwphysx_agent *p = agents->agents;

	const float dt = t / (float)n_steps;
	while (n_steps-- > 0) {
		for (int i = missiles->size - 1; i >= 0; i--) {
			const int j = nearest_hit(agents,
									  missiles->missiles[i],
									  missile_agent_threshold);
			if (j >= 0) {
				hit(&p[j], missiles, i);
			} else {
				propagate(&missiles->missiles[i].r,
						  missiles->missiles[i].u,
						  &missiles->missiles[i].v,
						  world->missile_acceleration,
						  dt);
			}
		}

		fill_distance_buffer(p, buffer->s1, n_agents);
		for (int i = 0; i < n_agents; i++) {
			buffer->p[i] = p[i];
			propagate(&buffer->p[i].r,
					  buffer->p[i].u,
					  &buffer->p[i].v,
					  buffer->p[i].a,
					  dt);
		}
		fill_distance_buffer(buffer->p, buffer->s2, n_agents);

		int32_t k = 0;
		for (int i = 0; i < n_agents; i++) {
			for (int j = i + 1; j < n_agents; j++) {
				const int both_alive = p[i].hp > 0.F && p[j].hp > 0.F;
				const int too_close = buffer->s1[k] > agent_agent_threshold ||
									  buffer->s2[k] > agent_agent_threshold;
				const int distance_decreases = buffer->s1[k] < buffer->s2[k];

				if (both_alive && too_close && distance_decreases) {
					buffer->p[i] = p[i];
					buffer->p[j] = p[j];
					collide(&buffer->p[i], &buffer->p[j], world->restitution);
				}

				k += 1;
			}
		}

		struct twsfwphysx_agent *tmp = p;
		p = buffer->p;
		buffer->p = tmp;
	}

	agents->agents = p;

	free(bffr.p);
	free(bffr.s1);
	free(bffr.s2);
}

void twsfwphysx_turn_agent(struct twsfwphysx_agent *agent, float angle)
{
	rotate(&agent->u, agent->r, angle);
}

struct twsfwphysx_missile
twsfwphysx_launch_missile(const struct twsfwphysx_agent *agent,
						  const struct twsfwphysx_world *world)
{
	struct twsfwphysx_missile missile = { agent->r, agent->u, agent->v, -1 };

	const float distance = world->agent_radius + world->agent_radius * 1e-4F;
	rotate(&missile.r, missile.u, distance);

	return missile;
}

#endif

#ifdef __cplusplus
} // extern "C"
#endif
