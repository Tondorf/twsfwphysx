#!/usr/bin/env python

import os

import git
from Cython.Build import cythonize
from setuptools import Extension, find_packages, setup

with open("README.md", encoding="utf-8") as f:
    long_description = f.read()

ext_modules = [
    Extension(
        "binding",
        sources=[os.path.join("twsfwphysx", "binding.pyx")],
        include_dirs=[os.path.join("..", "include")],
        extra_compile_args=["-DTWSFWPHYSX_IMPLEMENTATION"],
        language="c",
    )
]

setup(
    name="twsfwphysx",
    description="The physics engine for twsfw",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/tondorf/twsfwphysx",
    author="Happy Twondorfler",
    author_email="",  # TODO
    version=str(git.Repo("..").tags[-1]).lstrip("v"),
    packages=find_packages(exclude=["docs", "tests"]),
    python_requires=">=3.12",
    install_requires=[],
    extras_require={
        "dev": [
            "GitPython",
            "pytest",
            "ruff",
            "sphinx",
            "sphinx-rtd-theme",
            "sphinx-autodoc-typehints",
        ],
    },
    ext_modules=cythonize(ext_modules),
    zip_safe=False,
    include_package_data=True,
)
