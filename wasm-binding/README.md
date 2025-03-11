# twsfwphysx as a WASM module

This repository contains the source code that generates a WebAssembly (WASM) module
from [the twsfwphysx C-API](../include/twsfwphysx/twsfwphysx.h). This WASM module exposes a set of functions to manage
and simulate the propagation of world states. The generated WASM file is automatically built during continuous
integration (CI) and is attached as a [release artifact `twsfwphysx.wasm`][2].

Due to the API limitations of WASM modules, the world state has to be serialized into a buffer that was previously
allocated by the WASM module using `new_state_buffer()`. We use [FlatBuffers][1] for the (de)serialization and compile
language bindings for various languages during CI. Find the FlatBuffers schema [here][3] and
the language bindings attached as [release artifacts][2].

The following functions are exposed from the WASM module:

## Version Functions

These functions allow you to query the version of the WASM module:

- **`int32_t version_major()`**  
  Returns the major version number.
- **`int32_t version_minor()`**  
  Returns the minor version number.
- **`int32_t version_patch()`**  
  Returns the patch version number.

## World Initialization

- **`void init_world(float restitution, float agent_radius, float missile_acceleration)`**  
  Sets the world properties. See [the documentation of twsfwphysx_world](https://tondorf.github.io/twsfwphysx/) for more
  details:
    - `restitution`: The restitution coefficient.
    - `agent_radius`: The radius of agents in the simulation.
    - `missile_acceleration`: The acceleration value for missiles.

## State Buffer Management

- **`uint8_t *new_state_buffer(int32_t n_bytes)`**  
  Allocates a new state buffer of `n_bytes` bytes and returns a pointer to the first byte. This buffer is intended to
  hold the world state serialized as [FlatBuffers][1]. Use [our schema][3] to serialize the world state and then pass it
  to `simulate()`.

## Simulation Execution

- **`uint8_t *simulate(float t, int32_t n_steps, const uint8_t *state_buffer)`**  
  Runs the simulation and returns a pointer to a new state buffer containing the updated world state (also serialized as
  [FlatBuffers][1]).
    - `t`: Total simulation time.
    - `n_steps`: Number of simulation steps.
    - `state_buffer`: A pointer to the current world state; allocated by `new_state_buffer()` and serialized
      as [FlatBuffers][1]. Internally, `simulate()` uses the [schema][3] that was released with the version returned by
      the exposed `version_*()` functions.

[1]: https://flatbuffers.dev/

[2]: https://github.com/Tondorf/twsfwphysx/releases

[3]: ../twsfwphysx_world_state.fbs 

