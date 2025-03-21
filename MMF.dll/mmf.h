#pragma once
#include "pch.h"
#include <fstream>
#include <windows.h>
#include <string>
#include <algorithm>

#define LOG_FILE "log.txt"
#define SHARED_MEM_NAME "Local\\MySharedMemory"
#define MUTEX_NAME "Local\\MyMutex"
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

        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            hMutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, MUTEX_NAME);
            if (hMutex == NULL) {
                LogMessage("[ERROR] InitSharedMemory: Failed to open existing mutex" + std::to_string(GetLastError()));
                return false;
            }
        }

        hMapFile = CreateFileMappingA(
            INVALID_HANDLE_VALUE,   // ���������� ��������� ������
            NULL,                   // ���������� ������������
            PAGE_READWRITE,         // ������ �� ������/������
            0,                      // ������ � ������� 32 ����� (�� �����)
            SHARED_MEM_SIZE,        // ������ ������
            SHARED_MEM_NAME         // ���
        );

        if (hMapFile == NULL) {
            LogMessage("[ERROR] InitSharedMemory: Failed to create file mapping" + std::to_string(GetLastError()));
            CloseHandle(hMutex);
            return false;
        }

        pBuffer = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, SHARED_MEM_SIZE);
        if (pBuffer == nullptr) {
            LogMessage("[ERROR] InitSharedMemory: Failed to map view of file" + std::to_string(GetLastError()));
            CloseHandle(hMapFile);
            CloseHandle(hMutex);
            return false;
        }

        return true;
    }

    __declspec(dllexport) bool WriteData(const char* data, size_t size) {
        if (pBuffer == nullptr) {
            LogMessage("[ERROR] WriteData: ������: pBuffer == nullptr! �������� InitSharedMemory..." + std::to_string(GetLastError()));
            if (!InitSharedMemory()) {
                LogMessage("[ERROR] WriteData: ������ ��������� �������������!" + std::to_string(GetLastError()));
                return false;
            }
        }

        DWORD waitResult = WaitForSingleObject(hMutex, INFINITE);
        if (waitResult != WAIT_OBJECT_0) {
            LogMessage("[ERROR] WriteData: ������ �������� ��������" + std::to_string(GetLastError()));
            return false;
        }

        LogMessage("[INFO] WriteData: ������ ������ � ������...");
        memset(pBuffer, 0, SHARED_MEM_SIZE);
        memcpy(pBuffer, data, size);

        ReleaseMutex(hMutex);
        LogMessage("[INFO] WriteData: ������ ������� ��������");
        return true;
    }

    __declspec(dllexport) bool ReadData(char* buffer, size_t bufferSize) {
        if (!buffer || bufferSize == 0) {
            LogMessage("[ERROR] ReadData: �������� ��������� (buffer = nullptr ��� bufferSize = 0)");
            return false;
        }
        // ��������� ������� (��������� � C#)
        if (!hMutex) {
            hMutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, MUTEX_NAME);
            if (!hMutex) {
                LogMessage("[ERROR] ReadData: �� ������� ������� �������, ��� ������: " + std::to_string(GetLastError()));
                return false;
            }
        }

        // ��������� ����������� ������ (��������� � C#)
        if (!hMapFile) {
            hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEM_NAME);
            if (!hMapFile) {
                LogMessage("[ERROR] ReadData: �� ������� ������� file mapping, ��� ������: " + std::to_string(GetLastError()));
                return false;
            }
        }

        // ���������� ������, ���� ��� ��� �� �������
        if (!pBuffer) {
            pBuffer = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, SHARED_MEM_SIZE);
            if (!pBuffer) {
                LogMessage("[ERROR] ReadData: �� ������� ���������� ������, ��� ������: " + std::to_string(GetLastError()));
                return false;
            }
        }

        DWORD waitResult = WaitForSingleObject(hMutex, INFINITE);
        if (waitResult != WAIT_OBJECT_0) {
            LogMessage("[ERROR] ReadData: �� ������� ������������� �������, ��� ������: " + std::to_string(GetLastError()));
            return false;
        }

        size_t bytesToCopy = (bufferSize < SHARED_MEM_SIZE) ? bufferSize : SHARED_MEM_SIZE;
        memcpy(buffer, pBuffer, bytesToCopy);

        ReleaseMutex(hMutex);
        LogMessage("[INFO] ReadData: ������� ��������� " + std::to_string(bytesToCopy) + " ����");
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