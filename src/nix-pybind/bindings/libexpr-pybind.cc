#include "bindings.hh"

namespace py = pybind11;

void nix_gc_finalizer_trampoline(void * obj, void * cd)
{
    if (!cd)
        return; // Ensure callback data is valid

    auto * callback_data = static_cast<CallbackData *>(cd);

    try {
        pybind11::gil_scoped_acquire gil;                                // Ensure GIL is held when calling Python
        callback_data->py_func(py::cast(obj), callback_data->user_data); // Call Python function
    } catch (const py::error_already_set & e) {
        PyErr_WriteUnraisable(e.type().ptr());
    }

    delete callback_data; // Clean up memory after finalizer is called
}

void nix_gc_register_finalizer_wrapper(py::object obj, py::function py_callback, py::object user_data = py::none())
{
    auto * callback_data = new CallbackData{py_callback, user_data};
    nix_gc_register_finalizer(obj.ptr(), callback_data, nix_gc_finalizer_trampoline);
}

nix_err nix_eval_state_builder_set_lookup_path_wrapper(
    nix_c_context * context, nix_eval_state_builder * builder, py::list lookupPath)
{

    // Convert py::list to std::vector<std::string>
    std::vector<std::string> str_lookupPath;
    std::vector<const char *> c_lookupPath;

    for (py::handle item : lookupPath) {
        str_lookupPath.push_back(py::cast<std::string>(item)); // Store actual strings
        c_lookupPath.push_back(str_lookupPath.back().c_str()); // Store char* pointers
    }

    c_lookupPath.push_back(nullptr); // Null-terminate the array

    // Call the original API function
    return nix_eval_state_builder_set_lookup_path(context, builder, c_lookupPath.data());
}

EvalState * nix_state_create_wrapper(nix_c_context * context, py::list lookupPath, Store * store)
{
    // Convert py::list to std::vector<std::string>
    std::vector<std::string> str_lookupPath;
    std::vector<const char *> c_lookupPath;

    for (py::handle item : lookupPath) {
        str_lookupPath.push_back(py::cast<std::string>(item)); // Store actual strings
        c_lookupPath.push_back(str_lookupPath.back().c_str()); // Store char* pointers
    }

    c_lookupPath.push_back(nullptr); // Null-terminate the array

    // Call the original API function
    return nix_state_create(context, c_lookupPath.data(), store);
}

nix_err
nix_value_call_multi_wrapper(nix_c_context * context, EvalState * state, py::object fn, py::list args, py::object value)
{
    // Convert `fn` and `value` from Python to C++ pointers
    nix_value * fn_ptr = py::cast<nix_value *>(fn);
    nix_value * value_ptr = py::cast<nix_value *>(value);

    // Convert `args` (Python list) to C++ array of `nix_value*`
    size_t nargs = args.size();
    std::vector<nix_value *> args_vec;
    for (size_t i = 0; i < nargs; ++i) {
        args_vec.push_back(py::cast<nix_value *>(args[i]));
    }

    // Call the C++ function with the correct arguments
    return nix_value_call_multi(context, state, fn_ptr, nargs, args_vec.data(), value_ptr);
}

void init_libexpr(py::module_ & m)
{

    // define class EvalState as a struct opaque pointer
    // m.attr("EvalState") = py::capsule((void*)nullptr, static_cast<PyCapsule_Destructor>(nullptr));
    py::class_<nix_value>(m, "nix_value").def(py::init<>());
    py::class_<EvalState>(m, "EvalState");

    py::class_<nix_eval_state_builder>(m, "nix_eval_state_builder");

    /* Header nix_api_expr.h */
    m.def("nix_libexpr_init", &nix_libexpr_init, "Initialize the Nix language evaluator");
    // nix_err nix_expr_eval_from_string(nix_c_context * context, EvalState * state, const char * expr, const char *
    // path, nix_value * value);
    m.def("nix_expr_eval_from_string", &nix_expr_eval_from_string, "Evaluate a Nix expression from a string");
    // nix_err nix_value_call(nix_c_context * context, EvalState * state, nix_value * fn, nix_value * arg, nix_value *
    // value);
    m.def("nix_value_call", &nix_value_call, "Call a Nix function with an argument");
    // nix_err nix_value_call_multi(nix_c_context * context, EvalState * state, nix_value * fn, size_t nargs, nix_value
    // ** args, nix_value * value);
    m.def("nix_value_call_multi", &nix_value_call_multi_wrapper, "Call a Nix function with multiple arguments");
    // nix_err nix_value_force(nix_c_context * context, EvalState * state, nix_value * value);
    m.def("nix_value_force", &nix_value_force, "Force the evaluation of a Nix value");
    // nix_err nix_value_force_deep(nix_c_context * context, EvalState * state, nix_value * value);
    m.def("nix_value_force_deep", &nix_value_force_deep, "Force the deep evaluation of a Nix value");
    // nix_eval_state_builder * nix_eval_state_builder_new(nix_c_context * context, Store * store);
    m.def(
        "nix_eval_state_builder_new", &nix_eval_state_builder_new, "Create a new Nix language evaluator state builder");
    // nix_err nix_eval_state_builder_load(nix_c_context * context, nix_eval_state_builder * builder);
    m.def("nix_eval_state_builder_load", &nix_eval_state_builder_load, "Read settings from the ambient environment");
    // nix_err nix_eval_state_builder_set_lookup_path(nix_c_context * context, nix_eval_state_builder * builder, const
    // char ** lookupPath);
    m.def(
        "nix_eval_state_builder_set_lookup_path",
        &nix_eval_state_builder_set_lookup_path_wrapper,
        "Set the lookup path for `<...>` expressions");
    // EvalState * nix_eval_state_build(nix_c_context * context, nix_eval_state_builder * builder);
    m.def("nix_eval_state_build", &nix_eval_state_build, "Create a new Nix language evaluator state");
    // void nix_eval_state_builder_free(nix_eval_state_builder * builder);
    m.def("nix_eval_state_builder_free", &nix_eval_state_builder_free, "Free a Nix language evaluator state builder");
    // EvalState * nix_state_create(nix_c_context * context, const char ** lookupPath, Store * store);
    m.def("nix_state_create", &nix_state_create_wrapper, "Create a new Nix language evaluator state");
    // void nix_state_free(EvalState * state);
    m.def("nix_state_free", &nix_state_free, "Free a Nix language evaluator state");
    // nix_err nix_gc_incref(nix_c_context * context, const void * object);
    m.def("nix_gc_incref", &nix_gc_incref, "Increment the garbage collector reference counter for the given object");
    // nix_err nix_gc_decref(nix_c_context * context, const void * object);
    m.def("nix_gc_decref", &nix_gc_decref, "Decrement the garbage collector reference counter for the given object");
    // void nix_gc_now();
    m.def("nix_gc_now", &nix_gc_now, "Run the garbage collector");
    // void nix_gc_register_finalizer(void * obj, void * cd, void (*finalizer)(void * obj, void * cd));
    m.def("nix_gc_register_finalizer", &nix_gc_register_finalizer_wrapper, "Register a finalizer for an object");
    /**/
}
