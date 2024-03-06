#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "include/sample.hpp"
#include "include/comparisons.hpp"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

using namespace std;
namespace py = pybind11;


PYBIND11_MODULE(fn5, m) {
    m.doc() =  R"pbdoc(
        Fast, scalable SNP distance calculation from disk.
        -----------------------
           Sample
           save
           load
           load_reference
           load_mask
           compute
    )pbdoc";
    #ifdef VERSION_INFO
        m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
    #else
        m.attr("__version__") = "dev";
    #endif
    py::class_<Sample>(m, "Sample", R"pbdoc(
        Sample object. Used to store sample data and compute distances.
        -----------------------
        )pbdoc")
        .def_readwrite("uuid", &Sample::uuid,  R"pbdoc(
        string: Sample UUID
        )pbdoc")
        .def_readwrite("A", &Sample::A,  R"pbdoc(
        set[int]: Positions where this sample is `A`, but the reference is not.
        )pbdoc")
        .def_readwrite("C", &Sample::C,  R"pbdoc(
        set[int]: Positions where this sample is `C`, but the reference is not.
        )pbdoc")
        .def_readwrite("G", &Sample::G,  R"pbdoc(
        set[int]: Positions where this sample is `G`, but the reference is not.
        )pbdoc")
        .def_readwrite("T", &Sample::T,  R"pbdoc(
        set[int]: Positions where this sample is `T`, but the reference is not.
        )pbdoc")
        .def_readwrite("N", &Sample::N,  R"pbdoc(
        set[int]: Positions where this sample is `N`, but the reference is not.
        )pbdoc")
        .def_readwrite("qc_pass", &Sample::qc_pass,  R"pbdoc(
        bool: Whether this sample passed QC. i.e has >80% ACGT
        )pbdoc")
        .def(py::init<vector<int>, vector<int>, vector<int>, vector<int>, vector<int>>(), R"pbdoc(
        Instanciate a Sample object from existing values.
        -----------------------

        Args:
            A (set[int]): Positions where this sample is `A`, but the reference is not.
            C (set[int]): Positions where this sample is `C`, but the reference is not.
            G (set[int]): Positions where this sample is `G`, but the reference is not.
            T (set[int]): Positions where this sample is `T`, but the reference is not.
            N (set[int]): Positions where this sample is `N`, but the reference is not.
        )pbdoc", py::arg("A"), py::arg("C"), py::arg("G"), py::arg("T"), py::arg("N"))
        .def(py::init<string, string, unordered_set<int>, string>(), R"pbdoc(
        Instanciate a Sample object from a FASTA
        -----------------------

        Args:
            filepath (str): Path to this sample's FASTA file.
            reference (str): String of the reference FASTA file. See `load_reference`.
            mask (set[int]): Set of masked positions. See `load_mask`.
            uuid (str): Sample ID.
        )pbdoc", py::arg("filepath"), py::arg("reference"), py::arg("mask"), py::arg("uuid"))
        .def("dist", &Sample::dist, R"pbdoc(
        Calculate the distance between this sample and the given sample.
        -----------------------

        Args:
            sample (fn5.Sample): Sample to compare to. It is up to the user to ensure these are the same species.
            cutoff (int): SNP threshold. Set to 999999 for effectively no cutoff.
        
        Returns:
            int: SNP distance. If returned distance = cutoff + 1, the sample was further away than the cutoff. 
        )pbdoc", py::arg("sample"), py::arg("cutoff"))
        .def("__repr__",
            [](const Sample &s) {
                return "<fn5.Sample '" + s.uuid + "'>";
            }
        );
    m.def("save", &save, R"pbdoc(
        Save a sample to disk.
        -----------------------

        Args:
            path (str): Base path to save to. Produces 5 files of <uuid>.A, <uuid>.C, ...
            sample (fn5.Sample): Sample to save
        )pbdoc", py::arg("path"), py::arg("sample"));
    m.def("load", &readSample, R"pbdoc(
        Load a sample from disk.
        -----------------------

        Args:
            path (str): Base path to load from. i.e <some path>/<uuid> will load sample <uuid>
        
        Returns:
            fn5.Sample: Loaded sample.
        )pbdoc", py::arg("path"));
    m.def("load_reference", &load_reference, R"pbdoc(
        Load the reference from FASTA.
        -----------------------

        Args:
            filename (str): Path to a reference FASTA file

        Returns:
            str: Single line string of the FASTA file
        )pbdoc", py::arg("filename"));
    m.def("load_mask", &load_mask, R"pbdoc(
        Load a line separated genome index as a mask.
        -----------------------

        Args:
            filename (str): Path to a line separated list of genome positions to mask.

        Returns:
            set[int]: Set of genome positions which should be masked.
        )pbdoc", py::arg("filename"));
    m.def("compute", &multi_matrix, R"pbdoc(
        Compute a distance matrix for the given samples.
        -----------------------

        Args:
            samples (list[fn5.Sample]): List of samples who's distances should be computed.
            thread_count (int, optional): Number of threads to use for computation. Defaults to 4.
            cutoff (int, optional): SNP cutoff to use. Defaults to 999999 for effectively no cutoff.

        Returns:
            list[tuple[str, str, int]]: List of pairwise distances. If a pairwise distance is missing, is was further than SNP cutoff. Tuple format: (sample1.uuid, sample2.uuid, sample1.dist(sample2, cutoff))
        )pbdoc", py::arg("samples") , py::arg("thread_count") = 4, py::arg("cutoff") = 999999);
}

