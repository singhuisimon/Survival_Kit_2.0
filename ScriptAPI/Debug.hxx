#pragma once

/**
 * @file Debug.hxx
 * @brief Exception handling macros for native and managed code interop.
 * @author Kuek Wei Jie
 * @date October 5, 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#include <stdexcept>
#include <iostream>

#define SAFE_NATIVE_CALL_BEGIN try {
#define SAFE_NATIVE_CALL_END                                       \
}                                                                  \
catch (const std::exception& e)                                    \
{                                                                  \
    std::cout << "Native Exception: " << e.what() << std::endl;    \
}                                                                  \
catch (System::Exception^ e)                                       \
{                                                                  \
    System::Console::WriteLine("Managed Exception: " + e->Message);\
}                                                                  \
catch (...)                                                        \
{                                                                  \
    System::Console::WriteLine("Unknown exception caught.");       \
}