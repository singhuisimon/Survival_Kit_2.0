#pragma once

#ifdef SCRIPTCORE_EXPORTS
#define SCRIPTCORE_API __declspec(dllexport)
#else
#define SCRIPTCORE_API __declspec(dllimport)
#endif
