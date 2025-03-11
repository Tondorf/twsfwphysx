#include <cassert>
#include <cstring>
#include <string>
#include <vector>

#include "twsfwphysx/twsfwphysx.h"
#include "twsfwphysx_world_state_generated.h"

#ifdef NDEBUG
#    error "Assertions must be enabled. Compile without defining NDEBUG."
#endif

extern "C"
{
extern int32_t version_major();
extern int32_t version_minor();
extern int32_t version_patch();

extern void init_world(float restitution,
                       float agent_radius,
                       float missile_acceleration);

extern uint8_t *new_state_buffer(int32_t n_bytes);

extern uint8_t *simulate(float t, int32_t n_steps, const uint8_t *state_buffer);
}  // extern "C"

namespace
{
void test_version()
{
    const auto major = version_major();
    const auto minor = version_minor();
    const auto patch = version_patch();

    auto version = std::to_string(major);
    version += '.' + std::to_string(minor);
    version += '.' + std::to_string(patch);

    assert(version == twsfwphysx_version());
}

int asserts_enabled()
{
    int32_t ret = -1;
    assert(++ret == 0);  // NOLINT(bugprone-assert-side-effect)
    return ret;
}
}  // namespace

int main(int /*argc*/, char ** /*argv*/)
{
    test_version();

    init_world(1.F, .1F, 2.F);

    flatbuffers::FlatBufferBuilder builder(1024);

    std::vector<flatbuffers::Offset<twsfwphysx::Agent>> agents_offsets;
    {
        const auto r1 = twsfwphysx::Vec{1.F, 0.F, 0.F};
        const auto r2 = twsfwphysx::Vec{0.F, 1.F, 0.F};
        const auto u = twsfwphysx::Vec{0.F, 0.F, 1.F};

        agents_offsets.emplace_back(
            twsfwphysx::CreateAgent(builder, &r1, &u, .1F, .1F, 5.F));
        agents_offsets.emplace_back(
            twsfwphysx::CreateAgent(builder, &r2, &u, .0F, .1F, 5.F));
    }
    auto agents = builder.CreateVector(agents_offsets);

    std::vector<flatbuffers::Offset<twsfwphysx::Missile>> missiles_offsets;
    {
        const auto r = twsfwphysx::Vec{0.F, -1.F, 0.F};
        const auto u1 = twsfwphysx::Vec{0.F, 0.F, 1.F};
        const auto u2 = twsfwphysx::Vec{0.F, 0.F, -1.F};

        missiles_offsets.emplace_back(
            twsfwphysx::CreateMissile(builder, &r, &u1, 2.F, 1));
        missiles_offsets.emplace_back(
            twsfwphysx::CreateMissile(builder, &r, &u2, 2.F, 2));
    }
    auto missiles = builder.CreateVector(missiles_offsets);

    uint8_t *state_buffer = nullptr;
    {
        const auto state =
            twsfwphysx::CreateWorldState(builder, agents, missiles);
        builder.Finish(state);

        const auto buffer_size = builder.GetSize();
        state_buffer = new_state_buffer(static_cast<int32_t>(buffer_size));
        std::memcpy(state_buffer, builder.GetBufferPointer(), buffer_size);
    }

    state_buffer = simulate(1.F, 1'000, state_buffer);
    const auto *state = twsfwphysx::GetWorldState(state_buffer);

    assert(state->agents()->size() == 2);
    assert(state->missiles()->size() == 1);

    const auto &agent1 = state->agents()->Get(0);
    const auto &agent2 = state->agents()->Get(1);
    assert(std::abs(agent1->hp() - 2.F) < 1e-6);
    assert(std::abs(agent2->hp() - 5.F) < 1e-6);

    assert(std::abs(agent1->v() - .1F) < 1e-6);
    assert(agent1->v() > agent2->v());
    assert(agent2->v() > 0.F);

    assert(state->missiles()->Get(0)->payload() == 2);

    return asserts_enabled();
}
