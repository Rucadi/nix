#include "bindings.hh"

namespace py = pybind11;

void init_libflake(py::module_ & m)
{
    py::class_<nix_flake_settings>(m, "nix_flake_settings");

    // nix_flake_settings * nix_flake_settings_new(nix_c_context * context);
    m.def(
        "nix_flake_settings_new",
        &nix_flake_settings_new,
        "Create a new nix_flake_settings initialized with default values.");

    // void nix_flake_settings_free(nix_flake_settings * settings);
    m.def(
        "nix_flake_settings_free",
        &nix_flake_settings_free,
        "Release the resources associated with a nix_flake_settings.");

    // nix_err nix_flake_init_global(nix_c_context * context, nix_flake_settings * settings);
    m.def("nix_flake_init_global", &nix_flake_init_global, "Register Flakes support process-wide.");
}
