#include "bindings.hh"
namespace py = pybind11;


int nix_setting_get_wrapper(nix_c_context* context, const char *key, py::object py_callback, py::object user_data = py::none()) {
    CallbackData cb_data{py_callback, user_data};
    return nix_setting_get(context, key, nix_get_string_callback_trampoline, &cb_data);
}

int nix_err_info_msg_wrapper(nix_c_context* context, const nix_c_context* read_context, py::object py_callback, py::object user_data = py::none()) {
    CallbackData cb_data{py_callback, user_data};
    return nix_err_info_msg(context, read_context, nix_get_string_callback_trampoline, &cb_data);
}

int nix_err_name_wrapper(nix_c_context* context, const nix_c_context* read_context, py::object py_callback, py::object user_data = py::none()) {
    CallbackData cb_data{py_callback, user_data};
    return nix_err_name(context, read_context, nix_get_string_callback_trampoline, &cb_data);
}

void init_libutil(py::module_ &m) {

    py::enum_<nix_err>(m, "NixErr")
        .value("NIX_OK", nix_err::NIX_OK)
        .value("NIX_ERR_UNKNOWN", nix_err::NIX_ERR_UNKNOWN)
        .value("NIX_ERR_OVERFLOW", nix_err::NIX_ERR_OVERFLOW)
        .value("NIX_ERR_KEY", nix_err::NIX_ERR_KEY)
        .value("NIX_ERR_NIX_ERROR", nix_err::NIX_ERR_NIX_ERROR)
        .export_values();  // Expose the values to Python

    py::class_<nix_c_context>(m, "nix_c_context")
        .def_readwrite("last_err_code", &nix_c_context::last_err_code)
        .def_readwrite("last_err", &nix_c_context::last_err)
        .def_readwrite("info", &nix_c_context::info)
        .def_readwrite("name", &nix_c_context::name);

    //nix_c_context * nix_c_context_create();
    m.def("nix_c_context_create", &nix_c_context_create, "Create a new nix context");
    //void nix_c_context_free(nix_c_context * context);
    m.def("nix_c_context_free", &nix_c_context_free, "Free a nix context");
    //nix_err nix_libutil_init(nix_c_context * context);
    m.def("nix_libutil_init", &nix_libutil_init, "Initialize the Nix library utilities");
    //nix_err nix_setting_get(nix_c_context * context, const char * key, nix_get_string_callback callback, void * user_data);
    m.def("nix_setting_get", &nix_setting_get_wrapper, "Get a setting from the Nix global configuration",
          py::arg("context"), py::arg("key"), py::arg("callback"), py::arg("user_data") = py::none());
    //nix_err nix_setting_set(nix_c_context * context, const char * key, const char * value);
    m.def("nix_setting_set", &nix_setting_set, "Set a setting in the Nix global configuration");

    //const char * nix_version_get();
    m.def("nix_version_get", &nix_version_get, "Get the Nix library version");
    //const char * nix_err_msg(nix_c_context * context, const nix_c_context * ctx, unsigned int * n);
    m.def("nix_err_msg", &nix_err_msg, "Get the most recent error message from a context");
    //nix_err nix_err_info_msg(nix_c_context * context, const nix_c_context * read_context, nix_get_string_callback callback, void * user_data);
    m.def("nix_err_info_msg", &nix_err_info_msg_wrapper, "Get the error message from errorInfo in a context");
    //nix_err nix_err_name(nix_c_context * context, const nix_c_context * read_context, nix_get_string_callback callback, void * user_data);
    m.def("nix_err_name", &nix_err_name_wrapper, "Get the error name from a context");
    //nix_err nix_err_code(const nix_c_context * read_context);
    m.def("nix_err_code", &nix_err_code, "Get the most recent error code from a context");
    //nix_err nix_set_err_msg(nix_c_context * context, nix_err err, const char * msg);
    m.def("nix_set_err_msg", &nix_set_err_msg, "Set an error message on a context");
    //void nix_clear_err(nix_c_context * context);
    m.def("nix_clear_err", &nix_clear_err, "Clear the error message from a context");

  
}

