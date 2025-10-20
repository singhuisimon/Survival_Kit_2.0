// ImportExport.h
#pragma once
#ifndef DLL_API
#   if defined DLL_API_EXPORT
#       define DLL_API __declspec(dllexport)
#   else
#       define DLL_API __declspec(dllimport)
#   endif
#endif