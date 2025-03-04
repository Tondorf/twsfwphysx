import math

import pytest

import twsfwphysx


def dot(a, b):
    return a.x * b.x + a.y * b.y + a.z * b.z


def test_engine():
    r1 = twsfwphysx.Vec(1.0, 0.0, 0.0)
    r2 = twsfwphysx.Vec(0.0, 1.0, 0.0)
    u = twsfwphysx.Vec(0.0, 0.0, 1.0)

    agent1 = twsfwphysx.Agent(r1, u, 1.0, 1.0, 5.0)
    agent2 = twsfwphysx.Agent(r2, u, 0.0, 0.0, 5.0)

    world = twsfwphysx.World(
        restitution=1.0, agent_radius=0.1, missile_acceleration=2.0
    )

    engine = twsfwphysx.Engine(world, [agent1, agent2])
    assert len(engine.agents) == 2
    assert len(engine.missiles) == 0

    assert engine.agents[0] == pytest.approx(agent1)
    assert engine.agents[1] == pytest.approx(agent2)

    engine.launch_missile(agent_idx=0)
    engine.launch_missile(agent_idx=1, v=world.missile_acceleration)

    assert len(engine.missiles) == 2
    assert engine.missiles[0].v == pytest.approx(engine.agents[0].v)
    assert engine.missiles[1].v == pytest.approx(world.missile_acceleration)

    for agent, missile in zip(engine.agents, engine.missiles, strict=True):
        assert missile.u == pytest.approx(engine.agents[0].u)
        assert dot(agent.r, missile.r) == pytest.approx(
            math.cos(world.agent_radius), abs=1e-5
        )

    engine.simulate(t=2, n_steps=2_000)
    assert len(engine.missiles) == 1
    assert engine.agents[0].hp == pytest.approx(5)
    assert engine.agents[1].hp == pytest.approx(2)

    engine.simulate(t=5, n_steps=5_000)
    assert len(engine.missiles) == 0
    assert engine.agents[0].hp == pytest.approx(2)
    assert engine.agents[1].hp == pytest.approx(2)

    assert engine.agents[0].v > 0
    assert engine.agents[1].v > 0
