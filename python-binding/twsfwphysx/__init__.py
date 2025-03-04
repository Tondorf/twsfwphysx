"""Python Binding for the twsfwphysx engine."""

import binding
from binding import (
    Agent,  # noqa: F401
    Agents,  # noqa: F401
    Engine,  # noqa: F401
    Missile,  # noqa: F401
    Missiles,  # noqa: F401
    Vec,  # noqa: F401
    World,  # noqa: F401
)

__twsfwphysx_version__ = binding.get_twsfwphysx_version()
__version__ = __twsfwphysx_version__
