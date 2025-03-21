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
            INVALID_HANDLE_VALUE,   // Используем анонимную память
            NULL,                   // Дескриптор безопасности
            PAGE_READWRITE,         // Доступ на чтение/запись
            0,                      // Размер в старших 32 битах (не нужен)
            SHARED_MEM_SIZE,        // Размер памяти
            SHARED_MEM_NAME         // Имя
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
            LogMessage("[ERROR] WriteData: Ошибка: pBuffer == nullptr! Вызываем InitSharedMemory..." + std::to_string(GetLastError()));
            if (!InitSharedMemory()) {
                LogMessage("[ERROR] WriteData: Ошибка повторной инициализации!" + std::to_string(GetLastError()));
                return false;
            }
        }

        DWORD waitResult = WaitForSingleObject(hMutex, INFINITE);
        if (waitResult != WAIT_OBJECT_0) {
            LogMessage("[ERROR] WriteData: Ошибка ожидания мьютекса" + std::to_string(GetLastError()));
            return false;
        }

        LogMessage("[INFO] WriteData: Запись данных в память...");
        memset(pBuffer, 0, SHARED_MEM_SIZE);
        memcpy(pBuffer, data, size);

        ReleaseMutex(hMutex);
        LogMessage("[INFO] WriteData: Данные успешно записаны");
        return true;
    }

    __declspec(dllexport) bool ReadData(char* buffer, size_t bufferSize) {
        if (!buffer || bufferSize == 0) {
            LogMessage("[ERROR] ReadData: Неверные аргументы (buffer = nullptr или bufferSize = 0)");
            return false;
        }
        // Открываем мьютекс (созданный в C#)
        if (!hMutex) {
            hMutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, MUTEX_NAME);
            if (!hMutex) {
                LogMessage("[ERROR] ReadData: Не удалось открыть мьютекс, код ошибки: " + std::to_string(GetLastError()));
                return false;
            }
        }

        // Открываем разделяемую память (созданную в C#)
        if (!hMapFile) {
            hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEM_NAME);
            if (!hMapFile) {
                LogMessage("[ERROR] ReadData: Не удалось открыть file mapping, код ошибки: " + std::to_string(GetLastError()));
                return false;
            }
        }

        // Отображаем память, если это ещё не сделано
        if (!pBuffer) {
            pBuffer = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, SHARED_MEM_SIZE);
            if (!pBuffer) {
                LogMessage("[ERROR] ReadData: Не удалось отобразить память, код ошибки: " + std::to_string(GetLastError()));
                return false;
            }
        }

        DWORD waitResult = WaitForSingleObject(hMutex, INFINITE);
        if (waitResult != WAIT_OBJECT_0) {
            LogMessage("[ERROR] ReadData: Не удалось заблокировать мьютекс, код ошибки: " + std::to_string(GetLastError()));
            return false;
        }

        size_t bytesToCopy = (bufferSize < SHARED_MEM_SIZE) ? bufferSize : SHARED_MEM_SIZE;
        memcpy(buffer, pBuffer, bytesToCopy);

        ReleaseMutex(hMutex);
        LogMessage("[INFO] ReadData: Успешно прочитано " + std::to_string(bytesToCopy) + " байт");
        return true;
    }

    // Очистка ресурсов
    __declspec(dllexport) void Cleanup() {
        if (pBuffer) {
            if (UnmapViewOfFile(pBuffer)) {
                LogMessage("[INFO] Cleanup: Успешно отключено отображение памяти");
            }
            else {
                LogMessage("[ERROR] Cleanup: Ошибка при UnmapViewOfFile, код ошибки: " + std::to_string(GetLastError()));
            }
            pBuffer = nullptr;
        }

        if (hMapFile) {
            if (CloseHandle(hMapFile)) {
                LogMessage("[INFO] Cleanup: Дескриптор файла памяти закрыт");
            }
            else {
                LogMessage("[ERROR] Cleanup: Ошибка при CloseHandle(hMapFile), код ошибки: " + std::to_string(GetLastError()));
            }
            hMapFile = NULL;
        }

        if (hMutex) {
            if (CloseHandle(hMutex)) {
                LogMessage("[INFO] Cleanup: Мьютекс закрыт");
            }
            else {
                LogMessage("[ERROR] Cleanup: Ошибка при CloseHandle(hMutex), код ошибки: " + std::to_string(GetLastError()));
            }
            hMutex = NULL;
        }
    }
}