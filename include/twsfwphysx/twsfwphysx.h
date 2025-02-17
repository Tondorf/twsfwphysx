/*!
  \mainpage twsfwphysx Documentation

  Welcome to the twsfwphysx library documentation.

  For a detailed documentation of the API see \ref twsfwphysx.h.
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
 * @brief Returns the version of the library
 */
const char *twsfwphysx_version(void);

struct twsfwphysx_vec {
	float x;
	float y;
	float z;
};

struct twsfwphysx_particle {
	int16_t id;
	int16_t hp;
	float a;
	struct twsfwphysx_vec r;
	struct twsfwphysx_vec L;
};

struct twsfwphysx_particles {
	struct twsfwphysx_particle *particles;
	int32_t size;
	int32_t capacity;
};

struct twsfwphysx_missile {
	struct twsfwphysx_vec r;
	struct twsfwphysx_vec L;
};

struct twsfwphysx_missiles {
	struct twsfwphysx_missile *missiles;
	int32_t size;
	int32_t capacity;
};

struct twsfwphysx_world {
	float friction;
	float restitution;
	float particle_radius;
	float missile_acceleration;
};

struct twsfwphysx_simulation_buffer;

/**
 * @brief Creates a batch of new particles
 *
 * @param size Number of particles
 * @return Batch of particles
 */
struct twsfwphysx_particles twsfwphysx_create_particles(int32_t size);

/**
 * @brief Deletes all particles in a batch
 *
 * @param particles Particles
 */
void twsfwphysx_delete_particles(struct twsfwphysx_particles *particles);

/**
 * @brief Initializes particles at index
 *
 * @param particles Batch of particles
 * @param particle Initialization
 * @param index Index
 */
void twsfwphysx_init_particles(const struct twsfwphysx_particles *particles,
							   struct twsfwphysx_particle particle,
							   int32_t index);

/**
 * @brief Create a new batch of missiles
 *
 * @return The missile batch
 */
struct twsfwphysx_missiles twsfwphysx_new_missile_batch(void);

/**
 * @brief Deletes missile batch
 *
 * @param missiles
 */
void twsfwphysx_delete_missile_batch(struct twsfwphysx_missiles *missiles);

/**
 * @brief Adds a new missile to the batch
 *
 * @param missiles The missile batch
 * @param missile New missile
 */
void twsfwphysx_add_missile(struct twsfwphysx_missiles *missiles,
							struct twsfwphysx_missile missile);

/**
 * @brief Creates a new simulation buffer
 *
 * @return A new simulation buffer
 */
struct twsfwphysx_simulation_buffer twsfwphysx_create_simulation_buffer(void);

/**
 * @brief Deletes a simulation buffer
 */
void twsfwphysx_delete_simulation_buffer(
	struct twsfwphysx_simulation_buffer *buffer);

/**
 * @brief Simulates particle movements
 * @param particles Particles
 * @param missiles Missiles
 * @param world World
 * @param t Simulation time
 * @param n_steps Number of steps
 * @param buffer Simulation buffer
 */
void twsfwphysx_simulate(struct twsfwphysx_particles *particles,
						 struct twsfwphysx_missiles *missiles,
						 const struct twsfwphysx_world *world,
						 float t,
						 int32_t n_steps,
						 struct twsfwphysx_simulation_buffer *buffer);

#ifdef TWSFWPHYSX_IMPLEMENTATION

const char *twsfwphysx_version(void)
{
	return "0.1.0";
}

struct twsfwphysx_particles twsfwphysx_create_particles(const int32_t size)
{
	struct twsfwphysx_particles particles = { NULL, 0, 0 };
	if (size < 1) {
		return particles;
	}

	particles.particles =
		malloc((uint64_t)size * sizeof(struct twsfwphysx_particle));
	particles.size = size;
	particles.capacity = size;

	return particles;
}

void twsfwphysx_delete_particles(struct twsfwphysx_particles *particles)
{
	free(particles->particles);
	particles->particles = NULL;
	particles->size = 0;
	particles->capacity = 0;
}

void twsfwphysx_init_particles(const struct twsfwphysx_particles *particles,
							   const struct twsfwphysx_particle particle,
							   const int32_t index)
{
	assert(index >= 0 && index < particles->size);
	assert(particles->particles != NULL);
	particles->particles[index] = particle;
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
		missiles->missiles = realloc(missiles->missiles,
									 (uint64_t)missiles->capacity *
										 sizeof(struct twsfwphysx_missile));
		assert(missiles->missiles != NULL);
	}

	missiles->missiles[missiles->size++] = missile;
}

static float vec_length(const struct twsfwphysx_vec v)
{
	return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
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

static void rotate(struct twsfwphysx_vec *r,
				   const struct twsfwphysx_vec u,
				   const float theta)
{
	const float udr = dot(u, *r);
	const struct twsfwphysx_vec uxr = cross(u, *r);
	const struct twsfwphysx_vec uxr_x_u = cross(uxr, u);

	const float sin_theta = sinf(theta);
	const float cos_theta = cosf(theta);

	r->x = udr * u.x + cos_theta * uxr_x_u.x + sin_theta * uxr.x;
	r->y = udr * u.y + cos_theta * uxr_x_u.y + sin_theta * uxr.y;
	r->z = udr * u.z + cos_theta * uxr_x_u.z + sin_theta * uxr.z;
}

static void propagate(
	struct twsfwphysx_vec *r, // NOLINT(bugprone-easily-swappable-parameters)
	struct twsfwphysx_vec *L, // NOLINT(bugprone-easily-swappable-parameters)
	const float a, // NOLINT(bugprone-easily-swappable-parameters)
	const float dt, // NOLINT(bugprone-easily-swappable-parameters)
	const float mu) // NOLINT(bugprone-easily-swappable-parameters)
{
	const float x = mu * dt;
	float f;
	float g;

	if (fabsf(x) < 1e-5F) {
		// clang-format off
        f = -1.F / 24.F + x / 120.F;
        f =  1.F /  6.F + x * f;
        f = -1.F /  2.F + x * f;
        f =         1.F + x * f;

        g = -1.F / 120.F + x / 720.F;
        g =  1.F /  24.F + x * g;
        g = -1.F /   6.F + x * g;
        g =  1.F /   2.F + x * g;
        g = dt * g;
		// clang-format on
	} else {
		f = -expm1f(-x) / x;
		g = mu > 1e-5F ? (1.F - f) / mu : dt * (1.F - f) / x;
	}

	const struct twsfwphysx_vec v = cross(*L, *r);
	float v_abs = vec_length(v);
	const struct twsfwphysx_vec u = { L->x / v_abs,
									  L->y / v_abs,
									  L->z / v_abs };

	const float theta = (v_abs * f + a * g) * dt;

	v_abs = v_abs * expf(-x) + a * dt * f;
	L->x = u.x * v_abs;
	L->y = u.y * v_abs;
	L->z = u.z * v_abs;

	rotate(r, u, theta);
}

static void collide(struct twsfwphysx_particle *p1,
					struct twsfwphysx_particle *p2,
					const float epsilon)
{
	struct twsfwphysx_vec n = { p1->r.x - p2->r.x,
								p1->r.y - p2->r.y,
								p1->r.z - p2->r.z };

	const float length = vec_length(n);
	n.x /= length;
	n.y /= length;
	n.z /= length;

	struct twsfwphysx_vec v1 = cross(p1->L, p1->r);
	struct twsfwphysx_vec v2 = cross(p2->L, p2->r);
	const struct twsfwphysx_vec dv = { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
	const float J = (1.F + epsilon) / 2.F * dot(n, dv);

	v1.x = v1.x - J * n.x;
	v1.y = v1.y - J * n.y;
	v1.z = v1.z - J * n.z;

	v2.x = v2.x + J * n.x;
	v2.y = v2.y + J * n.y;
	v2.z = v2.z + J * n.z;

	p1->L = cross(p1->r, v1);
	p2->L = cross(p2->r, v2);
}

static void fill_distance_buffer(const struct twsfwphysx_particle *particles,
								 float *buffer,
								 int32_t n)
{
	assert(particles != NULL);
	assert(buffer != NULL);

	int32_t k = 0;
	for (int i = 0; i < n; i++) {
		for (int j = i + 1; j < n; j++) {
			buffer[k++] = dot(particles[i].r, particles[j].r);
		}
	}
}

static void hit(struct twsfwphysx_particle *particle,
				struct twsfwphysx_missiles *missiles,
				const int32_t i)
{
	const struct twsfwphysx_missile missile = missiles->missiles[i];
	const float cos_theta = dot(particle->L, missile.L) /
							vec_length(particle->L) / vec_length(missile.L);
	const long damage = lroundf(2.F + cos_theta);

	particle->hp = (int16_t)(particle->hp - damage);

	missiles->size -= 1;
	if (i < missiles->size) {
		missiles->missiles[i] = missiles->missiles[missiles->size];
	}
}

static int32_t nearest_hit(const struct twsfwphysx_particles *particles,
						   const struct twsfwphysx_missile missile,
						   const float threshold)
{
	int32_t i_max = -1;
	float s_max = -2.F; // -1 <= dot(.) <= +1
	for (int32_t i = 0; i < particles->size; i++) {
		const float s = dot(particles->particles[i].r, missile.r);
		if (s > threshold && s > s_max) {
			i_max = i;
			s_max = s;
		}
	}

	return i_max;
}

struct twsfwphysx_simulation_buffer {
	struct twsfwphysx_particle *p;
	float *s1;
	float *s2;
	int32_t capacity;
};

struct twsfwphysx_simulation_buffer twsfwphysx_create_simulation_buffer(void)
{
	const struct twsfwphysx_simulation_buffer buffer = { NULL, NULL, NULL, 0 };
	return buffer;
}

void twsfwphysx_delete_simulation_buffer(
	struct twsfwphysx_simulation_buffer *buffer)
{
	free(buffer->p);
	free(buffer->s1);
	free(buffer->s2);
	buffer->capacity = 0;
}

static struct twsfwphysx_simulation_buffer
update_simulation_buffer(struct twsfwphysx_simulation_buffer buffer,
						 const int32_t n_particles)
{
	assert(buffer.capacity >= 0);

	if (n_particles > buffer.capacity) {
		buffer.capacity = n_particles;

		const uint64_t n = (uint64_t)n_particles;

		// NOLINTNEXTLINE(bugprone-suspicious-realloc-usage)
		buffer.p = realloc(buffer.p, n * sizeof(struct twsfwphysx_particle));
		assert(buffer.p != NULL);

		if (n_particles > 1) {
			const uint64_t size = (n * (n - 1)) / 2 * sizeof(float);

			// NOLINTNEXTLINE(bugprone-suspicious-realloc-usage)
			buffer.s1 = realloc(buffer.s1, size);
			assert(buffer.s1 != NULL);

			// NOLINTNEXTLINE(bugprone-suspicious-realloc-usage)
			buffer.s2 = realloc(buffer.s2, size);
			assert(buffer.s2 != NULL);
		}
	}

	return buffer;
}

void twsfwphysx_simulate(struct twsfwphysx_particles *particles,
						 struct twsfwphysx_missiles *missiles,
						 const struct twsfwphysx_world *world,
						 const float t,
						 int32_t n_steps,
						 struct twsfwphysx_simulation_buffer *buffer)
{
	struct twsfwphysx_simulation_buffer bffr =
		twsfwphysx_create_simulation_buffer();
	if (buffer == NULL) {
		buffer = &bffr;
	}

	const int32_t n_particles = particles->size;
	update_simulation_buffer(*buffer, n_particles);

	// !!! WARNING !!!
	// cos(.) makes small angles large and large angles small!
	// Hence, search for distances *above* `threshold` when looking for
	// *close* objects.
	const float threshold = cosf(2.F * world->particle_radius);

	struct twsfwphysx_particle *p = particles->particles;

	const float dt = t / (float)n_steps;
	while (n_steps-- > 0) {
		for (int i = missiles->size - 1; i >= 0; i--) {
			const int j =
				nearest_hit(particles, missiles->missiles[i], threshold);
			if (j >= 0) {
				hit(&p[j], missiles, i);
			} else {
				propagate(&missiles->missiles[i].r,
						  &missiles->missiles[i].L,
						  world->missile_acceleration,
						  dt,
						  world->friction);
			}
		}

		fill_distance_buffer(p, buffer->s1, n_particles);
		for (int i = 0; i < n_particles; i++) {
			buffer->p[i] = p[i];
			propagate(&buffer->p[i].r,
					  &buffer->p[i].L,
					  buffer->p[i].a,
					  dt,
					  world->friction);
		}
		fill_distance_buffer(buffer->p, buffer->s2, n_particles);

		int32_t k = 0;
		for (int i = 0; i < n_particles; i++) {
			for (int j = i + 1; j < n_particles; j++) {
				const int both_alive = p[i].hp > 0 && p[j].hp > 0;
				const int too_close = buffer->s1[k] > threshold ||
									  buffer->s2[k] > threshold;
				const int distance_decreases = buffer->s1[k] < buffer->s2[k];

				if (both_alive && too_close && distance_decreases) {
					buffer->p[i] = p[i];
					buffer->p[j] = p[j];
					collide(&buffer->p[i], &buffer->p[j], world->restitution);
				}

				k += 1;
			}
		}

		struct twsfwphysx_particle *tmp = p;
		p = buffer->p;
		buffer->p = tmp;
	}

	particles->particles = p;
}

#endif

#ifdef __cplusplus
} // extern "C"
#endif
