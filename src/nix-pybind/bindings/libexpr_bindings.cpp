<#include <nix_api_util_internal.h>
#include <nix_api_expr_internal.h>
#include <nix_api_util.h>
#include <nix_api_store.h>
#include <nix_api_expr.h>
#include <nix_api_value.h>
#include <pybind11/stl.h>
#include <pybind11/pybind11.h>
namespace py = pybind11;


void init_libexpr(py::module_ &m) {

    py::class_<nix_c_context>(m, "nix_c_context")
        .def(py::init<>())
        .def_readwrite("last_err_code", &nix_c_context::last_err_code)
        .def_readwrite("last_err", &nix_c_context::last_err)
        .def_readwrite("info", &nix_c_context::info)
        .def_readwrite("name", &nix_c_context::name);

    py::class_<nix_value>(m, "nix_value")
        .def(py::init<>())
    
        
    // Binding the nix_err enum
    py::enum_<nix_err>(m, "NixErr")
        .value("NIX_OK", nix_err::NIX_OK)
        .value("NIX_ERR_UNKNOWN", nix_err::NIX_ERR_UNKNOWN)
        .value("NIX_ERR_OVERFLOW", nix_err::NIX_ERR_OVERFLOW)
        .value("NIX_ERR_KEY", nix_err::NIX_ERR_KEY)
        .value("NIX_ERR_NIX_ERROR", nix_err::NIX_ERR_NIX_ERROR)
        .export_values();  // Expose the values to Python


    // Now you can add the function that returns nix_err
    m.def("nix_libexpr_init", &nix_libexpr_init, "Initialize the Nix language evaluator");
}

