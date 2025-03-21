#include "mmf.h"
#include <windows.h>
#include <iostream>

static HANDLE hMapFile = NULL;
static LPVOID pBuf = NULL;
static std::string mmfName;

bool CreateMemoryMappedFile(const char* name, int size) {
    mmfName = name;
    hMapFile = CreateFileMappingA(
        INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, name);

    if (!hMapFile) {
        std::cerr << "Ошибка создания MMF: " << GetLastError() << std::endl;
        return false;
    }

    pBuf = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, size);
    if (!pBuf) {
        std::cerr << "Ошибка отображения MMF: " << GetLastError() << std::endl;
        CloseHandle(hMapFile);
        return false;
    }

    return true;
}

bool WriteToMemoryMappedFile(const char* message) {
    if (!pBuf) return false;
    strcpy_s((char*)pBuf, strlen(message) + 1, message);
    return true;
}

bool ReadFromMemoryMappedFile(char* buffer, int size) {
    if (!pBuf) return false;
    strncpy_s(buffer, size, (char*)pBuf, size);
    return true;
}

void CloseMemoryMappedFile() {
    if (pBuf) UnmapViewOfFile(pBuf);
    if (hMapFile) CloseHandle(hMapFile);
    pBuf = NULL;
    hMapFile = NULL;
}
