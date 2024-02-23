# Based on example https://github.com/pybind/python_example
from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup

__version__ = "1.0.0"

# The main interface is through Pybind11Extension.
# * You can add cxx_std=11/14/17, and then build_ext can be removed.
# * You can set include_pybind11=false to add the include directory yourself,
#   say from a submodule.
#
# Note:
#   Sort input source files if you glob sources to ensure bit-for-bit
#   reproducible builds (https://github.com/pybind/python_example/pull/53)

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
    # Currently, build_ext only provides an optional "highest supported C++
    # level" feature, but in the future it may provide more features.
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.7",
    license="University of Oxford, see LICENSE"
)