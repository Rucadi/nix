#include "bindings.hh"


PYBIND11_MODULE(nixpy, m) {
    m.doc() = "Python bindings for Nix C++ libraries";
   /*
    pybind11::module libflake = m.def_submodule("libflake", "Bindings for libflake-c");
    pybind11::module libstore = m.def_submodule("libstore", "Bindings for libstore-c");
    pybind11::module libutil = m.def_submodule("libutil", "Bindings for libutil-c");
    pybind11::module libexpr = m.def_submodule("libexpr", "Bindings for libexpr-c");
*/

    init_libflake(m);
    init_libstore(m);
    init_libutil(m);
    init_libexpr(m);
}


//REQUIRED FOR TESTING
#include <config-global.hh>
struct MySettings : nix::Config
{
nix::Setting<std::string> settingSet{this, "empty", "setting-name", "Description"};
};

MySettings mySettings;
static nix::GlobalConfig::Register rs(&mySettings);