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

void init_nix_api_expr(py::module_ & m)
{

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

nix_err nix_get_string_wrapper(
    nix_c_context * context, const nix_value * value, py::object py_callback, py::object user_data = py::none())
{
    CallbackData cb_data{py_callback, user_data};
    return nix_get_string(context, value, nix_get_string_callback_trampoline, &cb_data);
}

nix_value * py_nix_get_attr_byidx_wrapper(
    nix_c_context * context, const nix_value * value, EvalState * state, unsigned int i, py::object name_obj)
{
    // Ensure 'name' is a list
    if (!py::isinstance<py::list>(name_obj)) {
        throw std::invalid_argument("Error: 'name' parameter must be a list.");
    }

    py::list name = name_obj.cast<py::list>();

    const char * name_ptr = nullptr;

    // Call the original C++ function
    nix_value * result = nix_get_attr_byidx(context, value, state, i, &name_ptr);

    if (!result) {
        throw std::runtime_error("nix_get_attr_byidx returned null.");
    }

    // If name_ptr is valid, append or modify the list
    if (name_ptr) {
        if (name.empty()) {
            name.append(std::string(name_ptr)); // Append if list is empty
        } else {
            name[0] = std::string(name_ptr); // Modify first element if present
        }
    } else {
        if (name.empty()) {
            name.append(py::none()); // Append None if list is empty
        } else {
            name[0] = py::none(); // Modify first element to None
        }
    }

    return result;
}

void init_nix_api_value(py::module & m)
{
    py::enum_<ValueType>(m, "ValueType")
        .value("NIX_TYPE_THUNK", ValueType::NIX_TYPE_THUNK)
        .value("NIX_TYPE_INT", ValueType::NIX_TYPE_INT)
        .value("NIX_TYPE_FLOAT", ValueType::NIX_TYPE_FLOAT)
        .value("NIX_TYPE_BOOL", ValueType::NIX_TYPE_BOOL)
        .value("NIX_TYPE_STRING", ValueType::NIX_TYPE_STRING)
        .value("NIX_TYPE_PATH", ValueType::NIX_TYPE_PATH)
        .value("NIX_TYPE_NULL", ValueType::NIX_TYPE_NULL)
        .value("NIX_TYPE_ATTRS", ValueType::NIX_TYPE_ATTRS)
        .value("NIX_TYPE_LIST", ValueType::NIX_TYPE_LIST)
        .value("NIX_TYPE_FUNCTION", ValueType::NIX_TYPE_FUNCTION)
        .value("NIX_TYPE_EXTERNAL", ValueType::NIX_TYPE_EXTERNAL);

    py::class_<BindingsBuilder>(m, "BindingsBuilder");
    py::class_<ListBuilder>(m, "ListBuilder");
    py::class_<PrimOp>(m, "PrimOp");

    py::class_<nix_realised_string>(m, "nix_realised_string")
        .def("get_buffer_start", &nix_realised_string_get_buffer_start)
        .def("get_buffer_size", &nix_realised_string_get_buffer_size)
        .def("get_store_path_count", &nix_realised_string_get_store_path_count)
        .def("get_store_path", &nix_realised_string_get_store_path);

    m.def("nix_alloc_value", &nix_alloc_value, "Allocate a Nix value");

    m.def(
        "nix_value_incref",
        &nix_value_incref,
        "Increment the garbage collector reference counter for the given nix_value");

    m.def(
        "nix_value_decref",
        &nix_value_decref,
        "Decrement the garbage collector reference counter for the given nix_value");

    // Value manipulation functions
    m.def("nix_get_type", &nix_get_type, "Get value type");

    m.def("nix_get_typename", &nix_get_typename, "Get type name of value as defined in the evaluator");

    m.def("nix_get_bool", &nix_get_bool, "Get boolean value");

    m.def("nix_get_string", &nix_get_string_wrapper, "Get the raw string");

    m.def("nix_get_path_string", &nix_get_path_string, "Get path as string");

    m.def("nix_get_list_size", &nix_get_list_size, "Get the length of a list");

    m.def("nix_get_attrs_size", &nix_get_attrs_size, "Get the element count of an attrset");

    m.def("nix_get_float", &nix_get_float, "Get float value in 64 bits");

    m.def("nix_get_int", &nix_get_int, "Get int value");

    m.def("nix_get_external", &nix_get_external, "Get external reference");

    m.def("nix_get_list_byidx", &nix_get_list_byidx, "Get the ix'th element of a list");

    m.def("nix_get_attr_byname", &nix_get_attr_byname, "Get an attribute by index");

    m.def("nix_has_attr_byname", &nix_has_attr_byname, "Check if an attribute exists by name");

    m.def("nix_get_attr_byidx", &py_nix_get_attr_byidx_wrapper, " Get an attribute by index in the sorted bindings");

    m.def("nix_get_attr_name_byidx", &nix_get_attr_name_byidx, "Get the name of an attribute by index");

    m.def("nix_init_bool", &nix_init_bool, "Set boolean value");

    m.def("nix_init_string", &nix_init_string, "Set a string");

    m.def("nix_init_path_string", &nix_init_path_string, "Set a path");

    m.def("nix_init_float", &nix_init_float, "Set a float");

    m.def("nix_init_int", &nix_init_int, "Set an int");

    m.def("nix_init_null", &nix_init_null, "Set null");

    m.def("nix_init_apply", &nix_init_apply, "Apply a function to an argument");

    m.def("nix_init_external", &nix_init_external, "Set an external value");

    m.def("nix_make_list", &nix_make_list, "Create a list from a list builder");

    m.def("nix_make_list_builder", &nix_make_list_builder, "Create a list builder");

    m.def("nix_list_builder_insert", &nix_list_builder_insert, "Insert bindings into a builder");

    m.def("nix_list_builder_free", &nix_list_builder_free, "Free a list builder");

    m.def("nix_make_attrs", &nix_make_attrs, "Create an attribute set from a bindings builder");

    m.def("nix_init_primop", &nix_init_primop, "Initialize a PrimOp");

    m.def("nix_copy_value", &nix_copy_value, "Copy a Nix value");

    m.def("nix_make_bindings_builder", &nix_make_bindings_builder, "Create a new bindings builder");

    m.def("nix_bindings_builder_insert", &nix_bindings_builder_insert, "Insert bindings into a builder");

    m.def("nix_bindings_builder_free", &nix_bindings_builder_free, "Free a bindings builder");

    m.def("nix_string_realise", &nix_string_realise, "Realise a string context");

    m.def("nix_realised_string_get_buffer_start", &nix_realised_string_get_buffer_start, "Start of the string");

    m.def("nix_realised_string_get_buffer_size", &nix_realised_string_get_buffer_size, "Size of the string");

    m.def(
        "nix_realised_string_get_store_path_count", &nix_realised_string_get_store_path_count, "Number of store paths");

    m.def("nix_realised_string_get_store_path", &nix_realised_string_get_store_path, "Get a store path");

    m.def("nix_realised_string_free", &nix_realised_string_free, "Free a realised string"); /**/
}

void init_libexpr(py::module_ & m)
{
    py::class_<nix_value, std::unique_ptr<nix_value, py::nodelete>>(m, "nix_value");
    py::class_<EvalState, std::unique_ptr<EvalState, py::nodelete>>(m, "EvalState");
    py::class_<nix_eval_state_builder>(m, "nix_eval_state_builder");

    init_nix_api_expr(m);
    init_nix_api_value(m);
}