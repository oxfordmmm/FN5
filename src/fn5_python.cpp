#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "include/sample.hpp"
#include "include/comparisons.hpp"

using namespace std;
namespace py = pybind11;


PYBIND11_MODULE(fn5, m) {
    m.doc() = "Testing FN5 bindings";
    py::class_<Sample>(m, "Sample")
        .def_readwrite("uuid", &Sample::uuid)
        .def(py::init<vector<int>, vector<int>, vector<int>, vector<int>, vector<int>>())
        .def("dist", &Sample::dist);
    m.def("load", &readSample, "Load a sample from disk.");
    m.def("compute", &small_matrix, "Compute a matrix on a single thread.");
    m.def("multi_compute", &multi_matrix, "Compute a matrix multi-threaded.", py::arg("samples") , py::arg("thread_count") = 4);
}

