#pragma once
#include <config-store.hh>
#include <nix_api_util_internal.h>

#include <nix_api_expr_internal.h>
#include <nix_api_store_internal.h>
#include <nix_api_flake_internal.hh>
#include <nix_api_util.h>
#include <nix_api_expr.h>
#include <nix_api_store.h>
#include <nix_api_flake.h>

#include <pybind11/stl.h>
#include <pybind11/pybind11.h>



struct ExternalValue {

};

struct PrimOp {

};

void init_libflake(pybind11::module_ &);
void init_libutil(pybind11::module_ &);
void init_libexpr(pybind11::module_ &);
void init_libstore(pybind11::module_ &);


struct CallbackData
{
    pybind11::function py_func;
    pybind11::object user_data;
};

inline void nix_get_string_callback_trampoline(const char * start, unsigned int n, void * user_data)
{
    auto * cb_data = static_cast<CallbackData *>(user_data);
    if (cb_data) {
        try {
            pybind11::gil_scoped_acquire gil;
            cb_data->py_func(std::string(start, n), cb_data->user_data);
        } catch (const pybind11::error_already_set & e) {
            PyErr_Print();
        }
    }
}