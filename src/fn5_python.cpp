#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "include/sample.hpp"
#include "include/comparisons.hpp"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

using namespace std;
namespace py = pybind11;


PYBIND11_MODULE(fn5, m) {
    m.doc() = "Fast, scalable SNP distance calculation from disk.";
    #ifdef VERSION_INFO
        m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
    #else
        m.attr("__version__") = "dev";
    #endif
    py::class_<Sample>(m, "Sample")
        .def_readwrite("uuid", &Sample::uuid)
        .def_readwrite("A", &Sample::A)
        .def_readwrite("C", &Sample::C)
        .def_readwrite("G", &Sample::G)
        .def_readwrite("T", &Sample::T)
        .def_readwrite("N", &Sample::N)
        .def_readwrite("qc_pass", &Sample::qc_pass)
        .def(py::init<vector<int>, vector<int>, vector<int>, vector<int>, vector<int>>())
        .def(py::init<string, string, unordered_set<int>, string>())
        .def("dist", &Sample::dist)
        .def("__repr__",
            [](const Sample &s) {
                return "<fn5.Sample '" + s.uuid + "'>";
            }
        );
    m.def("save", &save, "Save a sample to disk.");
    m.def("load", &readSample, "Load a sample from disk.");
    m.def("load_reference", &load_reference, "Load the reference from FASTA.");
    m.def("load_mask", &load_mask, "Load a line separated genome index as a mask.");
    m.def("compute", &multi_matrix, "Compute a matrix multi-threaded.", py::arg("samples") , py::arg("thread_count") = 4, py::arg("cutoff") = 999999);
}

