#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#ifdef __EMSCRIPTEN__
#    include <emscripten.h>
#else
#    define EMSCRIPTEN_KEEPALIVE
#endif

// clang-format off
#define TWSFWPHYSX_IMPLEMENTATION
#include "twsfwphysx/twsfwphysx.h"
// clang-format on

#include "twsfwphysx_world_state_generated.h"

namespace
{
class Agents final
{
    twsfwphysx_agents m_agents;

  public:
    Agents()
        : m_agents(twsfwphysx_create_agents(0))
    {
    }

    Agents(Agents &&) = delete;
    Agents(const Agents &) = delete;
    Agents &operator=(const Agents &) = delete;
    Agents &operator=(Agents &&other) = delete;

    ~Agents()
    {
        twsfwphysx_delete_agents(&m_agents);
    }

    twsfwphysx_agent &operator[](const std::size_t idx)
    {
        if (const auto i = static_cast<int32_t>(idx); i >= m_agents.size) {
            const auto agents = twsfwphysx_create_agents(i + 1);
            for (int32_t j = 0; j < m_agents.size; j++) {
                twsfwphysx_set_agent(&agents, m_agents.agents[j], j);
            }
            twsfwphysx_delete_agents(&m_agents);

            m_agents = agents;
        }

        return m_agents.agents[idx];
    }

    [[nodiscard]] std::size_t size() const
    {
        return static_cast<std::size_t>(m_agents.size);
    }

    [[nodiscard]] twsfwphysx_agents *data()
    {
        return &m_agents;
    }
};

class Missiles final
{
    twsfwphysx_missiles m_missiles;

  public:
    Missiles()
        : m_missiles(twsfwphysx_new_missile_batch())
    {
    }

    Missiles(Missiles &&) = delete;
    Missiles(const Missiles &) = delete;
    Missiles &operator=(const Missiles &) = delete;
    Missiles &operator=(Missiles &&other) = delete;

    ~Missiles()
    {
        twsfwphysx_delete_missile_batch(&m_missiles);
    }

    void clear()
    {
        twsfwphysx_clear_missile_batch(&m_missiles);
    }

    void push_back(const twsfwphysx_missile &missile)
    {
        twsfwphysx_add_missile(&m_missiles, missile);
    }

    const twsfwphysx_missile &operator[](const std::size_t idx) const
    {
        const auto i = static_cast<int32_t>(idx);
        assert(i < m_missiles.size);
        return m_missiles.missiles[i];
    }

    [[nodiscard]] std::size_t size() const
    {
        return static_cast<std::size_t>(m_missiles.size);
    }

    [[nodiscard]] twsfwphysx_missiles *data()
    {
        return &m_missiles;
    }
};

twsfwphysx_world WORLD_CFG{
    .restitution = 1.F, .agent_radius = .1F, .missile_acceleration = 2.F};

Agents AGENTS;
Missiles MISSILES;

std::vector<flatbuffers::Offset<twsfwphysx::Agent>> AGENTS_OFFSETS;
std::vector<flatbuffers::Offset<twsfwphysx::Missile>> MISSILES_OFFSETS;

twsfwphysx_simulation_buffer *SIMULATION_BUFFER =
    twsfwphysx_create_simulation_buffer();

flatbuffers::FlatBufferBuilder FB_BUILDER(1024);
std::vector<uint8_t> STATE_BUFFER;

const std::array<int32_t, 3> VERSION = [](std::string version)
{
    assert(version.size() >= 5);

    std::size_t pos{};
    const int32_t major = std::stoi(version, &pos);

    assert(pos + 1 < version.size());
    version.erase(0, pos + 1);
    const int32_t minor = std::stoi(version, &pos);

    assert(pos + 1 < version.size());
    version.erase(0, pos + 1);
    const int32_t patch = std::stoi(version, &pos);

    assert(pos == version.size());

    return std::array{major, minor, patch};
}(twsfwphysx_version());

void deserialize(const uint8_t *state_buffer)
{
    const auto *state = twsfwphysx::GetWorldState(state_buffer);

    const auto n_agents = state->agents()->size();
    for (auto i = 0U; i < n_agents; i++) {
        const auto *agent = state->agents()->Get(i);
        AGENTS[i] = twsfwphysx_agent{.r = {.x = agent->r()->x(),
                                           .y = agent->r()->y(),
                                           .z = agent->r()->z()},
                                     .u = {.x = agent->u()->x(),
                                           .y = agent->u()->y(),
                                           .z = agent->u()->z()},
                                     .v = agent->v(),
                                     .a = agent->a(),
                                     .hp = agent->hp()};
    }

    MISSILES.clear();
    const auto n_missiles = state->missiles()->size();
    for (auto i = 0U; i < n_missiles; i++) {
        const auto *missile = state->missiles()->Get(i);
        MISSILES.push_back({.r = {.x = missile->r()->x(),
                                  .y = missile->r()->y(),
                                  .z = missile->r()->z()},
                            .u = {.x = missile->u()->x(),
                                  .y = missile->u()->y(),
                                  .z = missile->u()->z()},
                            .v = missile->v(),
                            .payload = missile->payload()});
    }
}

uint8_t *serialize()
{
    AGENTS_OFFSETS.clear();
    AGENTS_OFFSETS.reserve(AGENTS.size());
    for (auto i = 0U; i < AGENTS.size(); i++) {
        const auto &agent = AGENTS[i];
        const auto r = twsfwphysx::Vec{agent.r.x, agent.r.y, agent.r.z};
        const auto u = twsfwphysx::Vec{agent.u.x, agent.u.y, agent.u.z};
        AGENTS_OFFSETS.emplace_back(twsfwphysx::CreateAgent(
            FB_BUILDER, &r, &u, agent.v, agent.a, agent.hp));
    }
    const auto agents = FB_BUILDER.CreateVector(AGENTS_OFFSETS);

    MISSILES_OFFSETS.clear();
    MISSILES_OFFSETS.reserve(MISSILES.size());
    for (auto i = 0U; i < MISSILES.size(); i++) {
        const auto &missile = MISSILES[i];
        const auto r = twsfwphysx::Vec{missile.r.x, missile.r.y, missile.r.z};
        const auto u = twsfwphysx::Vec{missile.u.x, missile.u.y, missile.u.z};
        MISSILES_OFFSETS.emplace_back(twsfwphysx::CreateMissile(
            FB_BUILDER, &r, &u, missile.v, missile.payload));
    }
    const auto missiles = FB_BUILDER.CreateVector(MISSILES_OFFSETS);

    const auto state =
        twsfwphysx::CreateWorldState(FB_BUILDER, agents, missiles);
    FB_BUILDER.Finish(state);

    return FB_BUILDER.GetBufferPointer();
}
}  // namespace

extern "C"
{
EMSCRIPTEN_KEEPALIVE
int32_t version_major()
{
    return VERSION[0];
}

EMSCRIPTEN_KEEPALIVE
int32_t version_minor()
{
    return VERSION[1];
}

EMSCRIPTEN_KEEPALIVE
int32_t version_patch()
{
    return VERSION[2];
}

EMSCRIPTEN_KEEPALIVE
void init_world(const float restitution,
                const float agent_radius,
                const float missile_acceleration)
{
    ::WORLD_CFG.restitution = restitution;
    ::WORLD_CFG.agent_radius = agent_radius;
    ::WORLD_CFG.missile_acceleration = missile_acceleration;
}

EMSCRIPTEN_KEEPALIVE
uint8_t *new_state_buffer(const int32_t n_bytes)
{
    assert(n_bytes > 0);
    ::STATE_BUFFER.resize(static_cast<std::size_t>(n_bytes));

    return ::STATE_BUFFER.data();
}

EMSCRIPTEN_KEEPALIVE
uint8_t *simulate(const float t,
                  const int32_t n_steps,
                  const uint8_t *state_buffer)
{
    ::deserialize(state_buffer);

    twsfwphysx_simulate(AGENTS.data(),
                        MISSILES.data(),
                        &WORLD_CFG,
                        t,
                        n_steps,
                        SIMULATION_BUFFER);

    return ::serialize();
}

}  // extern "C"
