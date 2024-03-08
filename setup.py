# Based on example https://github.com/pybind/python_example
from pybind11.setup_helpers import Pybind11Extension
from setuptools import setup
from pathlib import Path
import os

__version__ = "v1.2.2"

try:
    this_directory = Path(__file__).parent
    long_description = (this_directory / "PYTHON_README.md").read_text()
except FileNotFoundError:
    # Issues around packaging other readme means we can just skip on install
    long_description = "Fast, scalable SNP distance calculation from disk."

# There's some odd behaviour going on here where pip won't install if this has the headers in it, 
#   but they're required to be distributed with the rest of the files
# So we have to run `python -m build` first including the headers, 
#   then again without before running `twine`
# So control with an env variable for automation
if os.environ.get("FN5_FIRST_PASS", None):
    files = sorted(["src/sample.cpp", "src/comparisons.cpp", "src/fn5_python.cpp", "src/include/sample.hpp", "src/include/comparisons.hpp"])
else:
    files = sorted(["src/sample.cpp", "src/comparisons.cpp", "src/fn5_python.cpp"])

ext_modules = [
    Pybind11Extension(
        "fn5",
        files,
        include_dirs=["src/include"],
        extra_compile_args=["-std=c++2a", "-O3", "-pthread"],
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
    ext_modules=ext_modules,
    zip_safe=False,
    python_requires=">=3.7",
    license="University of Oxford, see LICENSE",
    long_description=long_description,
    long_description_content_type='text/markdown',
    include_package_data=True,
    package_data={'fn5': ['./**']}
)