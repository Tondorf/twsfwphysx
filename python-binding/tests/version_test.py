import pathlib

import git

import twsfwphysx


def test_version():
    here = pathlib.Path(__file__).parent
    last_tag = git.Repo(here.parent.parent).tags[-1]
    assert str(last_tag).lstrip("v") == twsfwphysx.__version__


def test_twsfwphysx_version():
    assert twsfwphysx.__version__ == twsfwphysx.__twsfwphysx_version__
