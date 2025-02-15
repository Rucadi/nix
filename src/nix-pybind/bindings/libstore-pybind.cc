#include "bindings.hh"

namespace py = pybind11;

// Wrapper for nix_store_open
Store * nix_store_open_wrapper(nix_c_context * context, const std::string & uri, py::list params)
{
    // Convert py::list to std::vector<std::vector<std::string>>
    std::vector<std::vector<std::string>> str_params;
    std::vector<const char *> c_params;

    for (py::handle item : params) {
        auto kv_pair = py::cast<std::pair<std::string, std::string>>(item);
        str_params.push_back({kv_pair.first, kv_pair.second}); // Store key-value pair
        c_params.push_back(str_params.back()[0].c_str());      // Store key pointer
        c_params.push_back(str_params.back()[1].c_str());      // Store value pointer
    }

    c_params.push_back(nullptr); // Null-terminate the array

    // FIX: Remove the extra `&`
    return nix_store_open(context, uri.empty() ? nullptr : uri.c_str(), (const char ***) c_params.data());
}

nix_err nix_store_get_uri_wrapper(
    nix_c_context * context, Store * store, py::object py_callback, py::object user_data = py::none())
{
    CallbackData cb_data{py_callback, user_data};
    return nix_store_get_uri(context, store, nix_get_string_callback_trampoline, &cb_data);
}

nix_err nix_store_get_storedir_wrapper(
    nix_c_context * context, Store * store, py::object py_callback, py::object user_data = py::none())
{
    CallbackData cb_data{py_callback, user_data};
    return nix_store_get_storedir(context, store, nix_get_string_callback_trampoline, &cb_data);
}

void nix_store_path_name_wrapper(
    const StorePath * store_path, py::object py_callback, py::object user_data = py::none())
{
    CallbackData cb_data{py_callback, user_data};
    nix_store_path_name(store_path, nix_get_string_callback_trampoline, &cb_data);
}

// nix_err nix_store_realise(nix_c_context * context, Store * store, StorePath * path, void * userdata, void
// (*callback)(void * userdata, const char * outname, const char * out));

nix_err nix_store_real_path_wrapper(
    nix_c_context * context, Store * store, StorePath * path, py::object py_callback, py::object user_data = py::none())
{
    CallbackData cb_data{py_callback, user_data};
    return nix_store_real_path(context, store, path, nix_get_string_callback_trampoline, &cb_data);
}

nix_err nix_store_get_version_wrapper(
    nix_c_context * context, Store * store, py::object py_callback, py::object user_data = py::none())
{
    CallbackData cb_data{py_callback, user_data};
    return nix_store_get_version(context, store, nix_get_string_callback_trampoline, &cb_data);
}

void nix_store_realise_trampoline(void * userdata, const char * outname, const char * out)
{
    if (!userdata)
        return;

    auto * cb_data = static_cast<CallbackData *>(userdata);

    try {
        pybind11::gil_scoped_acquire gil;
        cb_data->py_func(outname, out, cb_data->user_data);
    } catch (const py::error_already_set & e) {
        PyErr_WriteUnraisable(e.type().ptr());
    }
}

nix_err nix_store_realise_wrapper(
    nix_c_context * context, Store * store, StorePath * path, py::object py_callback, py::object user_data = py::none())
{
    CallbackData cb_data{py_callback, user_data};
    return nix_store_realise(context, store, path, &cb_data, nix_store_realise_trampoline);
}

void init_libstore(py::module_ & m)
{
    py::class_<Store, std::unique_ptr<Store, py::nodelete>>(m, "Store");

    // nix_err nix_libstore_init(nix_c_context * context);
    m.def("nix_libstore_init", &nix_libstore_init);
    // nix_err nix_libstore_init_no_load_config(nix_c_context * context);
    m.def("nix_libstore_init_no_load_config", &nix_libstore_init_no_load_config);
    // Store * nix_store_open(nix_c_context * context, const char * uri, const char *** params);
    m.def("nix_store_open", &nix_store_open_wrapper, "Open a Nix store with optional URI and parameters");
    // void nix_store_free(Store * store);
    m.def("nix_store_free", &nix_store_free, "Free a Nix store");
    // nix_err nix_store_get_uri(nix_c_context * context, Store * store, nix_get_string_callback callback, void *
    // user_data);
    m.def("nix_store_get_uri", &nix_store_get_uri_wrapper, "Get the URI of a Nix store");
    // nix_err nix_store_get_storedir(nix_c_context * context, Store * store, nix_get_string_callback callback, void *
    // user_data);
    m.def(
        "nix_store_get_storedir",
        &nix_store_get_storedir_wrapper,
        "Get the storeDir of a Nix store, typically `\"/nix/store\"`");
    // StorePath * nix_store_parse_path(nix_c_context * context, Store * store, const char * path);
    m.def("nix_store_parse_path", &nix_store_parse_path, "Parse a Nix store path into a StorePath");
    // void nix_store_path_name(const StorePath * store_path, nix_get_string_callback callback, void * user_data);
    m.def("nix_store_path_name", &nix_store_path_name_wrapper, "Get the path name from a StorePath");
    // StorePath * nix_store_path_clone(const StorePath * p);
    m.def("nix_store_path_clone", &nix_store_path_clone, "Clone a StorePath");
    // void nix_store_path_free(StorePath * p);
    m.def("nix_store_path_free", &nix_store_path_free, "Free a StorePath");
    // bool nix_store_is_valid_path(nix_c_context * context, Store * store, StorePath * path);
    m.def("nix_store_is_valid_path", &nix_store_is_valid_path, "Check if a StorePath is valid");
    // nix_err nix_store_real_path(nix_c_context * context, Store * store, StorePath * path, nix_get_string_callback
    // callback, void * user_data);
    m.def("nix_store_real_path", &nix_store_real_path_wrapper, "Get the physical location of a store path");
    // nix_err nix_store_realise(nix_c_context * context, Store * store, StorePath * path, void * userdata, void
    // (*callback)(void * userdata, const char * outname, const char * out));
    m.def(
        "nix_store_realise",
        &nix_store_realise_wrapper,
        "Realise a Nix store path, invoking a callback with (outname, out, user_data).");

    // nix_err nix_store_get_version(nix_c_context * context, Store * store, nix_get_string_callback callback, void *
    // user_data);
    m.def("nix_store_get_version", &nix_store_get_version_wrapper, "Get the version of a Nix store");
    // nix_err nix_store_copy_closure(nix_c_context * context, Store * srcStore, Store * dstStore, StorePath * path);
    m.def(
        "nix_store_copy_closure", &nix_store_copy_closure, "Copy the closure of a StorePath from one store to another");
}
