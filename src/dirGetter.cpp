#include <ostream>
#include <string>
#include <vector>
#include "dirGetter.h"

#include <filesystem>
#include <iostream>
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

     std::string commandLine = std::string("cmd.exe /c \"dir /b \"") + path + "\"\"";

     // CreateProcessA necesita un buffer modificable
     std::vector<char> cmd(commandLine.begin(), commandLine.end());
     cmd.push_back('\0');
    if (!CreateProcessA(
      applicationName,
      cmd.data()             ,
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
            {
                bufferIndex++;
                continue;

            }
            if (c == '\n')
            {
                if (!constructionWord.empty())
                {
                    constructionWord.push_back('\0');
                    files.push_back(std::string(constructionWord.data()));
                }
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
std::string htmlEscape(const std::string& s)
 {
     std::string out;
     out.reserve(s.size());
     for (char c : s)
     {
         switch (c)
         {
         case '&':  out += "&amp;";  break;
         case '<':  out += "&lt;";   break;
         case '>':  out += "&gt;";   break;
         case '"':  out += "&quot;"; break;
         case '\'': out += "&#39;";  break;
         default:   out += c;        break;
         }
     }
     return out;
 }
 std::string url_decode(const std::string& value) {
     std::string result;
     char ch;
     int ii;

     size_t vLength = value.length();
     for (size_t i = 0; i < vLength; i++) {
         if (value[i] == '%') {
             sscanf_s(value.substr(i + 1, 2).c_str(), "%x", &ii);
             ch = static_cast<char>(ii);
             result += ch;
             i += 2;
         }
         else if (value[i] == '+') {
             result += ' ';
         }
         else {
             result += value[i];
         }
     }
     return result;
 }
inline std::string urlEncode(const std::string& s)
 {
     static auto hex = "0123456789ABCDEF";
     std::string out;
     for (unsigned char c : s)
     {
         if (std::isalnum(c) || c=='-' || c=='_' || c=='.' || c=='~' || c=='/')
             out += c;
         else { out += '%'; out += hex[c >> 4]; out += hex[c & 15]; }
     }
     return out;
 }
namespace fs = std::filesystem;
std::string recursiveHTMLFileSystemCreator(const char* path)
 {
     std::string result = "<ul class=\"tree\">\n";

    std::error_code ec;

    // u8path: interpreta el string como UTF-8 (no como codepage de Windows)
    std::filesystem::path(
    reinterpret_cast<const char8_t*>(path)
    );

     std::vector<std::string> availableFiles = {};
     if (!getFiles(path, availableFiles))
     {
         std::cerr << "Could not get files list: " << ec.message() << std::endl;
         return "";
     }
     for (auto file : availableFiles)
     {
         fs::path childPath = fs::path(path) / file;   // mantén el tipo fs::path
        try
        {


            childPath = childPath.u8string();
            if (fs::is_directory(childPath, ec))
            {
                result += "<li class=\"folder\"><details><summary>" + htmlEscape(file) + "</summary>"
                        + recursiveHTMLFileSystemCreator(childPath.string().c_str())
                        + "</details></li>\n";
            }else if (fs::is_regular_file(childPath, ec))
            {
                result += "<li><a href=\"" + htmlEscape(childPath.string()) + "\" download>" + htmlEscape(file) +"</a></li>";
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error with path: " << childPath.string() << " : " << e.what() << std::endl;
            continue;                   // sigue con el siguiente archivo
        }catch (const std::exception& e)
        {
            std::cerr << "Error inesperado: " << e.what() << "\n";
            continue;
        }
     }
     result += "</ul>\n";
     return result;
 }



void replaceAll(std::string& text, const std::string& key, const std::string& value)
 {
     size_t pos = 0;
     while ((pos = text.find(key, pos)) != std::string::npos)
     {
         text.replace(pos, key.size(), value);
         pos += value.size();   // salta lo insertado, evita bucle infinito
     }
 }

