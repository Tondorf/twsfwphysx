"""A Python Binding for the twsfwphysx engine.

This is a thin wrapper around the C-API of the
`twsfwphysx engine <https://github.com/Tondorf/twsfwphysx/>`_
that hides away manual memory management in RAII constructs and exposes the
engine state via canonical Python objects.

Start reading this documentation with :class:`Engine <twsfwphysx.Engine>`
and its :class:`Engine.simulate <twsfwphysx.Engine.simulate>` function. For
more general documentation on the twsfwphysx engine, read
`the documentation of the C-API <https://tondorf.github.io/twsfwphysx/>`_!
"""

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
