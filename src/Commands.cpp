//
// Created by migue on 04/10/2026.
//

#include "Commands.h"

#include <bemapiset.h>
#include <cstring>
#include <iostream>
#include <optional>
#include <windows.h>




void Commands::helpCMD()
{
    std::cout << "quit --> finish execution"<< std::endl;
    std::cout << "version --> print build version"<< std::endl;
    std::cout << "dir --> print your actual directory"<< std::endl;
    std::cout << "dir --> print your actual directory"<< std::endl;
}
void Commands::ls()
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    auto applicationName = "C://Windows//System32//cmd.exe";

    char commandLine[] = "/c dir";
    if (!CreateProcessA(
      applicationName,
      commandLine              ,
      NULL,NULL,
      FALSE,// ¿Heredar handles?
         0,                     // Flags de creación
        NULL,                  // Bloque de entorno
        NULL,                  // Directorio de trabajo actual
        &si,                   // Puntero a STARTUPINFO
        &pi                    // Puntero a PROCESS_INFORMATION
        ))
    {
        std::cerr << "Error execution ls command." << std::endl;
    }
}
void Commands::startCMD(char* exePath)
{

    std::cout << "write help to get help." << std::endl;
    std::cout << "write quit to get quit." << std::endl;

    while (true)
    {
        char userInput[MAX_USER_INPUT];
        std::cout << ">>" ;
        scanf("%s",userInput);
        if (strcmp(userInput, "quit") == 0)
        {


            break;
        } else if (strcmp(userInput, "help") == 0)
        {
            Commands::helpCMD();
        } else if (strcmp(userInput, "version") == 0)
        {
            std::cout << PROJECT_NAME << " version " << PROJECT_VERSION << std::endl;
        } else if (strcmp(userInput, "dir") == 0)
        {
            std::cout <<  exePath << std::endl;
        } else if (strcmp(userInput, "ls") == 0)
        {
            Commands::ls();
        }
    }

}
