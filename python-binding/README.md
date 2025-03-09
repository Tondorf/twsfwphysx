[![Python 3.12](https://img.shields.io/badge/python-3.12-blue.svg)](https://www.python.org/downloads/release/python-3120/)
[![Documentation](https://github.com/tondorf/twsfwphysx/actions/workflows/deploy_docs.yml/badge.svg)](https://tondorf.github.io/twsfwphysx/python-binding)
![Unit tests](https://github.com/tondorf/twsfwphysx/actions/workflows/validate_python_binding.yml/badge.svg)
[![Ruff](https://img.shields.io/endpoint?url=https://raw.githubusercontent.com/astral-sh/ruff/main/assets/badge/v2.json)](https://github.com/astral-sh/ruff)

# Python binding of twsfwphysx

This is a thin wrapper around the C-API of the [twsfwphysx engine](https://github.com/Tondorf/twsfwphysx/) that hides away manual memory management in RAII
constructs and exposes the engine state via canonical Python objects.
Find the documentation [here][1].

Install the Python package via:
```bash
pip install -e . 
```

or, if you want to install the optional developer dependencies as well:
```bash
pip install -e .[dev]
```

### Usage

```python
import twsfwphysx

print(f"{twsfwphysx.__version__=}")

from twsfwphysx import Agent, Engine, Vec, World
r = Vec(1., 0., 0.)
u = Vec(0., 0., 1.)
world = World(restitution=1., agent_radius=.1, missile_acceleration=2.)

engine = Engine(world, [Agent(r, u, 1., 1., 5.)])
engine.simulate(t=1, n_steps=1_000)
[...]
```
Checkout our [documentation][1] to learn more about what to do with an `engine`!

### Contributing

Contributing changes to this binding is easy! Make a pull-request and make sure that the code is properly formatted and
that the linters didn't find anything suspicious:

```bash
ruff format --check && ruff check
```

Then, run our test suite by simply typing

```bash
pytest
```

Make sure to run both commands in `twsfwphysx/python-binding` or adopt the paths accordingly. That's all!

[1]: https://tondorf.github.io/twsfwphysx/python-binding/
