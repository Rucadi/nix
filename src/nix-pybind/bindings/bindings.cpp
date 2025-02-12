#include <pybind11/pybind11.h>

/*void init_libexpr(pybind11::module_ &);
void init_libflake(pybind11::module_ &);
void init_libstore(pybind11::module_ &);*/
void init_libutil(pybind11::module_ &);

PYBIND11_MODULE(nixpy, m) {
    m.doc() = "Python bindings for Nix C++ libraries";
    
    /*pybind11::module libexpr = m.def_submodule("libexpr", "Bindings for libexpr-c");
    pybind11::module libflake = m.def_submodule("libflake", "Bindings for libflake-c");
    pybind11::module libstore = m.def_submodule("libstore", "Bindings for libstore-c");*/
    pybind11::module libutil = m.def_submodule("libutil", "Bindings for libutil-c");

   // init_libexpr(libexpr);
  /*  init_libflake(libflake);
    init_libstore(libstore);*/
    init_libutil(libutil);
}


//REQUIRED FOR TESTING
#include <config-global.hh>
struct MySettings : nix::Config
{
nix::Setting<std::string> settingSet{this, "empty", "setting-name", "Description"};
};

MySettings mySettings;
static nix::GlobalConfig::Register rs(&mySettings);