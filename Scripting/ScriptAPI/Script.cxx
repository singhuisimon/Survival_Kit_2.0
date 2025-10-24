#include "Script.hxx"
#include <stdexcept>
namespace ScriptAPI
{
    void Script::CreateNativeException() { throw std::runtime_error("Intentional Error!"); }
    void Script::CreateSEHException() { int *p = nullptr; *p = 42; }
}
