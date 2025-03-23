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
            INVALID_HANDLE_VALUE,   // Используем анонимную память
            NULL,                   // Дескриптор безопасности
            PAGE_READWRITE,         // Доступ на чтение/запись
            0,                      // Размер в старших 32 битах (не нужен)
            SHARED_MEM_SIZE,        // Размер памяти
            SHARED_MEM_NAME         // Имя
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
            LogMessage("[ERROR] WriteData: Ошибка: pBuffer == nullptr!" + std::to_string(GetLastError()));
            return false;
        }

        WaitForSingleObject(hMutex, INFINITE);
       
        LogMessage("[INFO] WriteData: Запись данных в память...");

        memset(pBuffer, 0, SHARED_MEM_SIZE);
        memcpy(pBuffer, &threadId, sizeof(int));  // Сохраняем ID потока
        memcpy((char*)pBuffer + sizeof(int), data, size);  // Записываем данные

        ReleaseMutex(hMutex);
        LogMessage("[INFO] WriteData: Данные успешно записаны");
        return true;
    }

    __declspec(dllexport) bool ReadData(int& threadId, char* buffer, size_t bufferSize) {
        hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEM_NAME);
        pBuffer = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, SHARED_MEM_SIZE);

        if (!pBuffer || !buffer || bufferSize == 0) {
            LogMessage("[ERROR] ReadData: Неверные аргументы (pBuffer = nullptr или buffer = nullptr или bufferSize = 0)");
            return false;
        }
        //// Открываем мьютекс (созданный в C#)
        //if (!hMutex) {
        //    hMutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, MUTEX_NAME);
        //    if (!hMutex) {
        //        LogMessage("[ERROR] ReadData: Не удалось открыть мьютекс, код ошибки: " + std::to_string(GetLastError()));
        //        return false;
        //    }
        //}

        //// Открываем разделяемую память (созданную в C#)
        //if (!hMapFile) {
        //    hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEM_NAME);
        //    if (!hMapFile) {
        //        LogMessage("[ERROR] ReadData: Не удалось открыть file mapping, код ошибки: " + std::to_string(GetLastError()));
        //        return false;
        //    }
        //}

        //// Отображаем память, если это ещё не сделано
        //if (!pBuffer) {
        //    pBuffer = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, SHARED_MEM_SIZE);
        //    if (!pBuffer) {
        //        LogMessage("[ERROR] ReadData: Не удалось отобразить память, код ошибки: " + std::to_string(GetLastError()));
        //        return false;
        //    }
        //}

        WaitForSingleObject(hMutex, INFINITE);
        
        memcpy(&threadId, pBuffer, sizeof(int));
        size_t dataSize = bufferSize < SHARED_MEM_SIZE - sizeof(int) ? bufferSize : SHARED_MEM_SIZE - sizeof(int);
        memcpy(buffer, (char*)pBuffer + sizeof(int), dataSize);

        ReleaseMutex(hMutex);
        LogMessage("[INFO] ReadData: Успешно прочитано");
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