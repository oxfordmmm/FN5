# Based on example https://github.com/pybind/python_example
from pybind11.setup_helpers import Pybind11Extension
from setuptools import setup

__version__ = "1.2.0"

ext_modules = [
    Pybind11Extension(
        "fn5",
        sorted(["src/sample.cpp", "src/comparisons.cpp", "src/fn5_python.cpp"]),
        include_dirs=["src/include"],
        extra_compile_args=["-std=c++20", "-O3", "-pthread"],
        # Example: passing in the version to the compiled code
        define_macros=[("VERSION_INFO", __version__)],
    ),
]

setup(
    name="fn5",
    version=__version__,
    author="Jeremy Westhead",
    author_email="jeremy.westhead@ndm.ox.ac.uk",
    url="https://github.com/oxfordmmm/FN5",
    description="Fast, scalable SNP distance calculation from disk.",
    long_description="",
    ext_modules=ext_modules,
    zip_safe=False,
    python_requires=">=3.7",
    license="University of Oxford, see LICENSE"
)