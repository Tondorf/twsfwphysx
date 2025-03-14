import collections

from dataclasses import dataclass
from math import pi

from libc.stdint cimport int32_t


cdef extern from "twsfwphysx/twsfwphysx.h":
    cdef struct twsfwphysx_vec:
        float x
        float y
        float z

    cdef struct twsfwphysx_agent:
        twsfwphysx_vec r
        twsfwphysx_vec u
        float v
        float a
        float hp

    cdef struct twsfwphysx_agents:
        twsfwphysx_agent *agents
        int32_t size

    cdef struct twsfwphysx_missile:
        twsfwphysx_vec r
        twsfwphysx_vec u
        float v
        int32_t payload

    cdef struct twsfwphysx_missiles:
        twsfwphysx_missile *missiles
        int32_t size
        int32_t capacity

    cdef struct twsfwphysx_world:
        float restitution
        float agent_radius
        float missile_acceleration

    const char* twsfwphysx_version()

    twsfwphysx_agents twsfwphysx_create_agents(int32_t size)

    void twsfwphysx_delete_agents(twsfwphysx_agents *agents)

    void twsfwphysx_set_agent(const twsfwphysx_agents *batch,
                              twsfwphysx_agent agent,
                              int32_t index)
    
    twsfwphysx_missiles twsfwphysx_new_missile_batch()

    void twsfwphysx_delete_missile_batch(twsfwphysx_missiles *missiles)

    twsfwphysx_missile twsfwphysx_launch_missile(const twsfwphysx_agent *agent,
                                                 const twsfwphysx_world *world)

    void twsfwphysx_add_missile(twsfwphysx_missiles *missiles,
                                twsfwphysx_missile missile)
                                
    void *twsfwphysx_create_simulation_buffer()

    void twsfwphysx_delete_simulation_buffer(void *buffer)

    void twsfwphysx_simulate(twsfwphysx_agents *agents,
                             twsfwphysx_missiles *missiles,
                             const twsfwphysx_world *world,
                             float t,
                             int32_t n_steps,
                             void *buffer)

    void twsfwphysx_turn_agent(twsfwphysx_agent *agent, float angle)


def get_twsfwphysx_version() -> str:
    cdef const char* version = twsfwphysx_version()
    return version.decode("utf-8")


@dataclass(frozen=True)
class World:
    """World invariants.

    The world invariants. See
    `the documentation of the C-API <https://tondorf.github.io/twsfwphysx/>_
    for more details.
    """

    restitution: float
    agent_radius: float
    missile_acceleration: float


@dataclass(frozen=True)
class Vec:
    """Representation of a 3D vector"""

    x: float
    y: float
    z: float


@dataclass(frozen=True)
class Agent:
    """Agent attributes."""

    r: Vec
    u: Vec
    v: float
    a: float
    hp: float


@dataclass(frozen=True)
class Missile:
    """Missile attributes."""

    r: Vec
    u: Vec
    v: float
    payload: int


class Agents:
    """View of agents.

    Use this view to iterate over a batch of :class:`Agent <twsfwphysx.Agent>.
    In order to change their attributes, replace agents at specific indices
    with a new instance of :class:`Agent <twsfwphysx.Agent>`.

    Do not try to create new instances of :class:`Agents <twsfwphysx.Agents>`
    manually but rather use :class:`Engine.agents <twsfwphysx.Engine.agents>`.
    """

    def __init__(self, engine):
        self._engine = engine

    def __len__(self):
        return self._engine._get_agents_size()

    def __getitem__(self, index: int):
        n = self._engine._get_agents_size()
        if index < 0 or index >= n:
            raise IndexError(
                f"Invalid index '{index}' (total number of agents: {n})"
            )

        return self._engine._get_agent(index)

    def __setitem__(self, index: int, agent: Agent):
        n = self._engine._get_agents_size()
        if index < 0 or index >= n:
            raise IndexError(
                f"Invalid index '{index}' (total number of agents: {n})"
            )

        return self._engine._set_agent(index, agent)


class Missiles:
    """View of missiles.

    Use this view to iterate over a batch of
    :class:`Missile <twsfwphysx.Missile>. In order to change their
    attributes, replace agents at specific indices with a new instance of
    :class:`Missile <twsfwphysx.Missile>`.

    Do not try to create new instances of
    :class:`Missiles <twsfwphysx.Missiles>` manually but rather use
    :class:`Engine.missiles <twsfwphysx.Engine.missiles>`.
    """

    def __init__(self, engine):
        self._engine = engine

    def __len__(self):
        return self._engine._get_missiles_size()

    def __getitem__(self, index: int):
        n = self._engine._get_missiles_size()
        if index < 0 or index >= n:
            raise IndexError(
                f"Invalid index '{index}' (total number of missiles: {n})"
            )

        return self._engine._get_missile(index)

    def __setitem__(self, index: int, missile: Missile):
        n = self._engine._get_missiles_size()
        if index < 0 or index >= n:
            raise IndexError(
                f"Invalid index '{index}' (total number of missiles: {n})"
            )

        return self._engine._set_missile(index, missile)


cdef class Engine:
    """Wrapper around the twsfwphysx Engine.

    This class is a wrapper around the twsfwphysx Engine. It holds a list of
    agents, missiles, and world invariants and passes those to the twsfwphysx
    simlatiom function once
    :class:`Engine.simulate <twsfwphysx.Engine.simulate>` is called.

    The number of agents cannot be changed after a new Engine was created. In
    order to change the number of missiles, use the function
    :class:`Engine.launch_missile <twsfwphysx.Engine.launch_missile>`.
    
    Use :class:`Engine.turn_agent <twsfwphysx.Engine.turn_agent>` to turn an
    agent and the variables :class:`Engine.agents <twsfwphysx.Engine.agents>`
    and :class:`Engine.missiles <twsfwphysx.Engine.missiles>` to get a view of
    the attributes of all agents and missiles, respectively.

    :param world: The world invariants.
    :type world: World
    :param agents: Agents with their initial attributes. Note that the number of agents cannot be changed once an Engine is created.
    :type agents: collections.abc.Iterable[Agent]
    """

    cdef twsfwphysx_world _world
    cdef twsfwphysx_agents _agents
    cdef twsfwphysx_missiles _missiles
    cdef void *_simulation_buffer

    def __init__(self, world: World, agents: collections.abs.Iterable[Agent]):
        self._world = twsfwphysx_world(
            world.restitution,
            world.agent_radius,
            world.missile_acceleration
        )

        self._agents = twsfwphysx_create_agents(len(agents))
        cdef twsfwphysx_agent *a
        for i, agent in enumerate(agents):
            a = &self._agents.agents[i]

            a.r.x = agent.r.x
            a.r.y = agent.r.y
            a.r.z = agent.r.z

            a.u.x = agent.u.x
            a.u.y = agent.u.y
            a.u.z = agent.u.z

            a.v = agent.v
            a.a = agent.a
            a.hp = agent.hp

        self._missiles = twsfwphysx_new_missile_batch()

        self._simulation_buffer = twsfwphysx_create_simulation_buffer()

    def __del__(self):
        twsfwphysx_delete_agents(&self._agents)
        twsfwphysx_delete_missile_batch(&self._missiles)

        if self._simulation_buffer:
            twsfwphysx_delete_simulation_buffer(self._simulation_buffer)

    def _check_agent_idx(self, idx: int):
        if idx < 0 or idx >= self._agents.size:
            raise IndexError(
                f"Invalid index '{idx}' (total number of agents: {self._agents.size})"
            )

    def simulate(self, *, t: float, n_steps: int):
        """Simulates the propagation and interactions of agents and missiles.

        Simulates the propagation and interactions of agents and missiles for
        `t` seconds (simulation time; hopefully much faster in real-time)
        using `n_steps` internal steps. The later increase the overall
        computation time but also increases the accuracy of the collision
        detection among agents but also between agents and missiles.
        As a rule of thumb, the propagation distance of agents and missiles
        during one simulation steps should be smaller than
        :class:`World.agent_radius <twsfwphysx.World.agent_radius>`.

        :param t: Simulation time in seconds. (Not real time!)
        :param n_steps: Number of internal simulation steps.
        """

        twsfwphysx_simulate(
            &self._agents,
            &self._missiles,
            &self._world,
            t,
            n_steps,
            self._simulation_buffer
        )

    def launch_missile(self,
                       *,
                       agent_idx: int,
                       v: Optional[float]=None,
                       payload: Optional[int]=None):
        """launch missile.

        Launches a missile next to the given agent. For internal reasons this
        function cannot be implemented as a member function of
        :class:`Agent <twsfwphysx.Agent>`. Hence, the index of the agent's
        position in :class:`Engine.agents <twsfwphysx.Engine.agents>` has to be
        given as the first argument.

        :param agent_idx: Index of agent in :class:`Engine.agents <twsfwphysx.Engine.agents>`.
        :param v: Initial velocity. If not given, the velocity of the agent is used.
        :param payload: An optional persistent payload.
        :raises IndexError: if `agent_idx` is invalid.
        """

        self._check_agent_idx(agent_idx)
        cdef twsfwphysx_agent agent = self._agents.agents[agent_idx]

        cdef twsfwphysx_missile missile = twsfwphysx_launch_missile(&agent, &self._world)

        if v is not None:
            missile.v = v

        if payload is None:
            missile.payload = agent_idx

        twsfwphysx_add_missile(&self._missiles, missile)

    def turn_agent(self, *, agent_idx: int, angle: float, degrees: bool=True):
        """Turns the agent.

        Turns the (angular momentum vector of) agent by the given angle. For
        internal reasons this helper function cannot be implemented as a
        member function of :class:`Agent <twsfwphysx.Agent>`. Hence, the index
        of the agent's position in :class:`Engine.agents <twsfwphysx.Engine.agents>`
        has to be given as the first argument.

        :param agent_idx: Index of agent in :class:`Engine.agents <twsfwphysx.Engine.agents>`.
        :param angle: Rotation angle in degrees (default) or radians.
        :param degrees: Unit of rotation angle. If set to `False`, the unit is radians.
        :raises IndexError: if `agent_idx` is invalid.
        """

        self._check_agent_idx(agent_idx)

        if degrees:
            angle /= 180. * pi

        twsfwphysx_turn_agent(&self._agents.agents[agent_idx], angle)
    
    def _get_agents_size(self):
        return self._agents.size

    def _get_agent(self, index: int):
        cdef twsfwphysx_agent a = self._agents.agents[index]

        r = Vec(a.r.x, a.r.y, a.r.z)
        u = Vec(a.u.x, a.u.y, a.u.z)

        return Agent(r, u, a.v, a.a, a.hp)

    def _set_agent(self, index: int, agent: Agent):
        cdef twsfwphysx_agent* a = &self._agents.agents[index]

        a.r.x = agent.r.x
        a.r.y = agent.r.y
        a.r.z = agent.r.z

        a.u.x = agent.u.x
        a.u.y = agent.u.y
        a.u.z = agent.u.z

        a.v = agent.v
        a.a = agent.a
        a.hp = agent.hp

    def _get_missiles_size(self):
        return self._missiles.size

    def _get_missile(self, index: int):
        cdef twsfwphysx_missile m = self._missiles.missiles[index]

        r = Vec(m.r.x, m.r.y, m.r.z)
        u = Vec(m.u.x, m.u.y, m.u.z)

        return Missile(r, u, m.v, m.payload)

    def _set_missile(self, index: int, missile: Missile):
        cdef twsfwphysx_missile* m = &self._missiles.missiles[index]

        m.r.x = missile.r.x
        m.r.y = missile.r.y
        m.r.z = missile.r.z

        m.u.x = missile.u.x
        m.u.y = missile.u.y
        m.u.z = missile.u.z

        m.v = missile.v

    @property
    def agents(self) -> Agents:
        """View of all agents.

        Use this view to iterate over agents or to change their attributes.

        :return: Agents.
        :rtype: Agents
        """

        return Agents(self)

    @property
    def missiles(self) -> Missiles:
        """View of all missiles.

        Use this view to iterate over missiles or to change their attributes.

        :return: Missiles.
        :rtype: Missiles
        """

        return Missiles(self)
