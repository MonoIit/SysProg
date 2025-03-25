#pragma once
#include "pch.h"
#include <fstream>
#include <windows.h>
#include <string>
#include <algorithm>
using namespace std;

#define LOG_FILE L"log.txt"
#define MUTEX_NAME "Global\\MyMutex"

HANDLE hMapFile = NULL;
HANDLE hMutex = NULL;

struct header
{
    int addr;
    int size;
};


wstring GetLastErrorString(DWORD ErrorID = 0)
{
    if (!ErrorID)
        ErrorID = GetLastError();
    if (!ErrorID)
        return wstring();

    LPWSTR pBuff = nullptr;
    size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, ErrorID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&pBuff, 0, NULL);
    wstring s(pBuff, size);
    LocalFree(pBuff);

    return s;
}


extern "C" {

    void LogMessage(const wstring& message) {
        std::wofstream log(LOG_FILE, std::ios::app);
        if (log.is_open()) {
            log << message << std::endl;
        }
    }

    __declspec(dllexport) bool InitSharedMemory() {
        hMutex = CreateMutexA(NULL, FALSE, MUTEX_NAME);
        if (hMutex == NULL) {
            LogMessage(L"[ERROR] InitSharedMemory: Failed to create mutex" + GetLastError());
            return false;
        }
        return true;
    }

    __declspec(dllexport) bool WriteData(int threadId, const wchar_t* data) {

    WaitForSingleObject(hMutex, INFINITE);

            header h = { threadId, int(wcslen(data) * 2) };
            HANDLE hFile = CreateFile(L"filemap.dat", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, 0);

            HANDLE hFileMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, h.size + sizeof(header), L"MyMap");
            BYTE* buff = (BYTE*)MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, h.size + sizeof(header));
            
       
            LogMessage(L"[INFO] WriteData: Запись данных в память...");



            memcpy(buff, &h, sizeof(header));

            memcpy(buff + sizeof(header), data, h.size);

            UnmapViewOfFile(buff);

            //	return hFileMap;

            CloseHandle(hFileMap);

            CloseHandle(hFile);

            ReleaseMutex(hMutex);
            LogMessage(L"[INFO] WriteData: Данные успешно записаны");
            return true;
    }

    __declspec(dllexport) wstring ReadData(header& h) {
        WaitForSingleObject(hMutex, INFINITE);

        HANDLE hFile = CreateFile(L"filemap.dat", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, 0);

        HANDLE hFileMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, sizeof(header), L"MyMap");

        LPVOID buff = MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(header));
        h = *((header*)buff);
        UnmapViewOfFile(buff);
        CloseHandle(hFileMap);

        int n = h.size + sizeof(header);
        hFileMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, n, L"MyMap");
        buff = MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, n);
        wstring s((wchar_t*)((BYTE*)buff + sizeof(header)), h.size / 2);

        UnmapViewOfFile(buff);
        CloseHandle(hFileMap);

        CloseHandle(hFile);

        ReleaseMutex(hMutex);
        LogMessage(L"[INFO] ReadData: Успешно прочитано");
        return s;
    }


    __declspec(dllexport) void Cleanup() {
        if (hMutex) {
            if (CloseHandle(hMutex)) {
                LogMessage(L"[INFO] Cleanup: Мьютекс закрыт");
            }
            else {
                LogMessage(L"[ERROR] Cleanup: Ошибка при CloseHandle(hMutex), код ошибки: " + std::to_wstring(GetLastError()));
            }
            hMutex = NULL;
        }
    }
}