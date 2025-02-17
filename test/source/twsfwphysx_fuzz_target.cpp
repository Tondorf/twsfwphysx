#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cmath>
#include <numbers>

#include "twsfwphysx/twsfwphysx.h"

namespace
{
[[nodiscard]] twsfwphysx_world make_world(const std::array<uint8_t, 4> &data)
{
	return twsfwphysx_world{
		.friction = static_cast<float>(data[0]) / 10.F,
		.restitution = static_cast<float>(data[1]) / 255.F,
		.agent_radius = static_cast<float>(data[2]) / 2550.F,
		.missile_acceleration = static_cast<float>(data[3]) / 10.F,
	};
}

[[nodiscard]] twsfwphysx_vec make_unit_vec(const std::array<uint8_t, 2> &data)
{
	const float lat =
		static_cast<float>(data[0]) / 255.F * std::numbers::pi_v<float> * 2.F;
	const float lon =
		static_cast<float>(data[1]) / 255.F * std::numbers::pi_v<float> * 2.F;

	return twsfwphysx_vec{ std::cos(lat) * std::cos(lon),
						   std::cos(lat) * std::sin(lon),
						   std::sin(lat) };
}

[[nodiscard]] twsfwphysx_vec make_vec(const std::array<uint8_t, 3> &data)
{
	const float r = (static_cast<float>(data[2]) + 1.F) / 10.F;
	auto v = make_unit_vec({ data[0], data[1] });
	v.x *= r;
	v.y *= r;
	v.z *= r;

	return v;
}

[[nodiscard]] twsfwphysx_agent make_agent(const std::array<uint8_t, 7> &data)
{
	return twsfwphysx_agent{ .r = make_unit_vec({ data[0], data[1] }),
							 .L = make_vec({ data[2], data[3], data[4] }),
							 .a = static_cast<float>(data[5]) / 255.F,
							 .hp = (static_cast<int32_t>(data[6]) - 5) / 50 };
}
[[nodiscard]] twsfwphysx_missile
make_missile(const std::array<uint8_t, 5> &data)
{
	return twsfwphysx_missile{ .r = make_unit_vec({ data[0], data[1] }),
							   .L = make_vec({ data[2], data[3], data[4] }) };
}

template <std::size_t N>
[[nodiscard]] std::array<uint8_t, N> data2array(const uint8_t data[])
{
	std::array<uint8_t, N> a;
	for (std::size_t i = 0; i < N; i++) {
		a[i] = data[i];
	}

	return a;
}
} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	if (size < 6) {
		return -1;
	}

	const auto world = make_world({ data[0], data[1], data[2], data[3] });
	const auto n_agents = static_cast<size_t>(data[4]);
	const auto n_missiles = static_cast<size_t>(data[5]);

	if (size < 6 + n_agents * 7 + n_missiles * 5) {
		return -1;
	}

	data = data + 6;

	auto agents = twsfwphysx_create_agents(static_cast<int32_t>(n_agents));
	for (size_t i = 0; i < n_agents; i++) {
		twsfwphysx_set_agent(&agents,
							 make_agent(data2array<7>(data)),
							 static_cast<int32_t>(i));
		data += 7;
	}

	auto missiles = twsfwphysx_new_missile_batch();
	for (size_t i = 0; i < n_missiles; i++) {
		twsfwphysx_add_missile(&missiles, make_missile(data2array<5>(data)));
		data += 5;
	}

	auto *buffer = twsfwphysx_create_simulation_buffer();

	twsfwphysx_simulate(&agents, &missiles, &world, 100, 1000, buffer);
	twsfwphysx_simulate(&agents, &missiles, &world, 100, 1000, buffer);

	twsfwphysx_delete_agents(&agents);
	twsfwphysx_delete_missile_batch(&missiles);
	twsfwphysx_delete_simulation_buffer(buffer);

	return 0;
}