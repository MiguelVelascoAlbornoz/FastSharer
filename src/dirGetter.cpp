#include <ostream>
#include <string>
#include <vector>
#include "dirGetter.h"
#include <windows.h>
//
// Created by migue on 05/10/2026.
//
 bool getFiles(const char* path, std::vector<std::string>& files)
{
    //Creacion de pipes, uno para lectura y otro de escritura
    HANDLE readPipe;
    HANDLE writePipe;

    //Configuracion para que puedan  ser heredables a procesos hijos
    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;

    //Creacion del pipe en si
    CreatePipe(&readPipe, &writePipe, &sa, 0);

    //Se configura el proceso hijo para que el stdout, stderr y stdin vallan a los pipes creados
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdOutput = writePipe;
    si.hStdError  = writePipe;
    si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));
    auto applicationName = "C://Windows//System32//cmd.exe";

    char commandLine[] = "/c dir /b";
    if (!CreateProcessA(
      applicationName,
      commandLine              ,
      NULL,NULL,
      TRUE,// ¿Heredar handles?
         0,                     // Flags de creación
        NULL,                  // Bloque de entorno
        NULL,                  // Directorio de trabajo actual
        &si,                   // Puntero a STARTUPINFO
        &pi                    // Puntero a PROCESS_INFORMATION
        ))
    {
        CloseHandle(readPipe);
        CloseHandle(writePipe);
        return false;
    }
    CloseHandle(writePipe);
    char buffer[4096];
    DWORD bytesRead;
    std::vector<char> constructionWord ={};
    while (ReadFile(readPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL)
           && bytesRead > 0)
    {
        buffer[bytesRead] = '\0';
        int bufferIndex = 0;
        while (buffer[bufferIndex])
        {
            char c = buffer[bufferIndex];
            if (c == '\r')
                continue;
                bufferIndex++;
            if (c == '\n')
            {
                if (!constructionWord.empty())
                    constructionWord.push_back('\0');
                    files.push_back(std::string(constructionWord.data()));
                bufferIndex++;
                constructionWord.clear();
                continue;
            }
            constructionWord.push_back(c);
            bufferIndex++;
        }
    }
    if (!constructionWord.empty())              // última línea sin salto final
        files.push_back(std::string(constructionWord.data()));

    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(readPipe);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
}