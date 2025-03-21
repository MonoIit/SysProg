#pragma once

#ifdef MYMMF_EXPORTS
#define MYMMF_API __declspec(dllexport)
#else
#define MYMMF_API __declspec(dllimport)
#endif

extern "C" {
    MYMMF_API bool CreateMemoryMappedFile(const char* name, int size);
    MYMMF_API bool WriteToMemoryMappedFile(const char* message);
    MYMMF_API bool ReadFromMemoryMappedFile(char* buffer, int size);
    MYMMF_API void CloseMemoryMappedFile();
}