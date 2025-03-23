#pragma once
#include "pch.h"
#include <fstream>
#include <windows.h>
#include <string>
#include <algorithm>

#define LOG_FILE "log.txt"
#define SHARED_MEM_NAME "Global\\MySharedMemory"
#define MUTEX_NAME "Global\\MyMutex"
#define SHARED_MEM_SIZE 4096

HANDLE hMapFile = NULL;
HANDLE hMutex = NULL;
void* pBuffer = nullptr;

extern "C" {

    void LogMessage(const std::string& message) {
        std::ofstream log(LOG_FILE, std::ios::app);
        if (log.is_open()) {
            log << message << std::endl;
        }
    }

    __declspec(dllexport) bool InitSharedMemory() {
        hMutex = CreateMutexA(NULL, FALSE, MUTEX_NAME);
        if (hMutex == NULL) {
            LogMessage("[ERROR] InitSharedMemory: Failed to create mutex" + std::to_string(GetLastError()));
            return false;
        }

        hMapFile = CreateFileMappingA(
            INVALID_HANDLE_VALUE,   // ���������� ��������� ������
            NULL,                   // ���������� ������������
            PAGE_READWRITE,         // ������ �� ������/������
            0,                      // ������ � ������� 32 ����� (�� �����)
            SHARED_MEM_SIZE,        // ������ ������
            SHARED_MEM_NAME         // ���
        );
    
        pBuffer = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, SHARED_MEM_SIZE);
        if (pBuffer == nullptr) {
            LogMessage("[ERROR] InitSharedMemory: Failed to map view of file" + std::to_string(GetLastError()));
            return false;
        }

        return true;
    }

    __declspec(dllexport) bool WriteData(int threadId, const char* data, size_t size) {
        if (!pBuffer || size > SHARED_MEM_SIZE - sizeof(int)) {
            LogMessage("[ERROR] WriteData: ������: pBuffer == nullptr!" + std::to_string(GetLastError()));
            return false;
        }

        WaitForSingleObject(hMutex, INFINITE);
       
        LogMessage("[INFO] WriteData: ������ ������ � ������...");

        memset(pBuffer, 0, SHARED_MEM_SIZE);
        memcpy(pBuffer, &threadId, sizeof(int));  // ��������� ID ������
        memcpy((char*)pBuffer + sizeof(int), data, size);  // ���������� ������

        ReleaseMutex(hMutex);
        LogMessage("[INFO] WriteData: ������ ������� ��������");
        return true;
    }

    __declspec(dllexport) bool ReadData(int& threadId, char* buffer, size_t bufferSize) {
        hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEM_NAME);
        pBuffer = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, SHARED_MEM_SIZE);

        if (!pBuffer || !buffer || bufferSize == 0) {
            LogMessage("[ERROR] ReadData: �������� ��������� (pBuffer = nullptr ��� buffer = nullptr ��� bufferSize = 0)");
            return false;
        }
        //// ��������� ������� (��������� � C#)
        //if (!hMutex) {
        //    hMutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, MUTEX_NAME);
        //    if (!hMutex) {
        //        LogMessage("[ERROR] ReadData: �� ������� ������� �������, ��� ������: " + std::to_string(GetLastError()));
        //        return false;
        //    }
        //}

        //// ��������� ����������� ������ (��������� � C#)
        //if (!hMapFile) {
        //    hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEM_NAME);
        //    if (!hMapFile) {
        //        LogMessage("[ERROR] ReadData: �� ������� ������� file mapping, ��� ������: " + std::to_string(GetLastError()));
        //        return false;
        //    }
        //}

        //// ���������� ������, ���� ��� ��� �� �������
        //if (!pBuffer) {
        //    pBuffer = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, SHARED_MEM_SIZE);
        //    if (!pBuffer) {
        //        LogMessage("[ERROR] ReadData: �� ������� ���������� ������, ��� ������: " + std::to_string(GetLastError()));
        //        return false;
        //    }
        //}

        WaitForSingleObject(hMutex, INFINITE);
        
        memcpy(&threadId, pBuffer, sizeof(int));
        size_t dataSize = bufferSize < SHARED_MEM_SIZE - sizeof(int) ? bufferSize : SHARED_MEM_SIZE - sizeof(int);
        memcpy(buffer, (char*)pBuffer + sizeof(int), dataSize);

        ReleaseMutex(hMutex);
        LogMessage("[INFO] ReadData: ������� ���������");
        return true;
    }

    // ������� ��������
    __declspec(dllexport) void Cleanup() {
        if (pBuffer) {
            if (UnmapViewOfFile(pBuffer)) {
                LogMessage("[INFO] Cleanup: ������� ��������� ����������� ������");
            }
            else {
                LogMessage("[ERROR] Cleanup: ������ ��� UnmapViewOfFile, ��� ������: " + std::to_string(GetLastError()));
            }
            pBuffer = nullptr;
        }

        if (hMapFile) {
            if (CloseHandle(hMapFile)) {
                LogMessage("[INFO] Cleanup: ���������� ����� ������ ������");
            }
            else {
                LogMessage("[ERROR] Cleanup: ������ ��� CloseHandle(hMapFile), ��� ������: " + std::to_string(GetLastError()));
            }
            hMapFile = NULL;
        }

        if (hMutex) {
            if (CloseHandle(hMutex)) {
                LogMessage("[INFO] Cleanup: ������� ������");
            }
            else {
                LogMessage("[ERROR] Cleanup: ������ ��� CloseHandle(hMutex), ��� ������: " + std::to_string(GetLastError()));
            }
            hMutex = NULL;
        }
    }
}