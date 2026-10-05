//
// Created by migue on 04/10/2026.
//

#include "Commands.h"

#include <bemapiset.h>
#include <charconv>
#include <cstring>
#include <iostream>
#include <optional>
#include <windows.h>

#include "dirGetter.h"

std::optional<int> toInt(const std::string& s)
{
    int value;
    const char* first = s.data();
    const char* last  = s.data() + s.size();

    auto [ptr, ec] = std::from_chars(first, last, value);

    if (ec != std::errc() || ptr != last)
        return std::nullopt;   // no es número, o desborda int, o sobran caracteres
    return value;
}
void Commands::setDepth(std::vector<std::string> &args)
{
    if (args.size() == 1)
    {
        std::cout << "Max recursion depth: " << maxHTMLRecursionDepth << std::endl;
    } else
    {
        if (auto n = toInt(args[1]))
        {
            maxHTMLRecursionDepth = n.value();
            std::cout << "Max recursion depth is setted to: " << maxHTMLRecursionDepth << std::endl;
        }

        else std::cout << "Argumento inválido\n";
    }
}
void Commands::breakCommand(const char* userInput, std::vector<std::string>& args)
{
    size_t actualPos = 0;
    std::string commandLine = std::string(userInput);
    size_t pos = commandLine.find(' ',actualPos);
    std::string stringToAdd = "";
    while (pos != std::string::npos && actualPos < commandLine.length())
    {
        stringToAdd = commandLine.substr(actualPos, pos - actualPos);
        if (stringToAdd != "")        args.push_back(stringToAdd);

        actualPos = pos + 1;
        pos = commandLine.find(' ',pos+1);
    }

    stringToAdd = commandLine.substr(actualPos, pos - actualPos);
    if (stringToAdd != "")        args.push_back(stringToAdd);
}

void Commands::helpCMD()
{
    std::cout << "quit --> finish execution"<< std::endl;
    std::cout << "version --> print build version"<< std::endl;
    std::cout << "dir --> print your actual directory"<< std::endl;
    std::cout << "dir --> print your actual directory"<< std::endl;
    std::cout << "max_depth <int>*--> prints/ change your max recursion depth while searching files"<< std::endl;
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
        std::cout << ">>" << std::flush;

        // lee hasta el salto de línea, máximo MAX_USER_INPUT-1 caracteres
        if (scanf("%[^\n]", userInput) != 1)
            userInput[0] = '\0';   // línea vacía o EOF

        // consume el '\n' que queda en el buffer
        scanf("%*c");

        std::vector<std::string> args;
        breakCommand(userInput, args);
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
        } else if (args.size() >= 1)
        {
            if (strcmp(args[0].c_str(), "max_depth") == 0)
            {
                Commands::setDepth(args);
            }
        }
    }

}
