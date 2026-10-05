#include <ostream>
#include <string>
#include <vector>
#include "dirGetter.h"


#include <algorithm>
#include <iostream>
#include <windows.h>
//
// Created by migue on 05/10/2026.
//
int maxHTMLRecursionDepth = 5;
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
    std::string word;
    char buffer[65536];
    DWORD bytesRead;

    while (ReadFile(readPipe, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0)
    {
        for (DWORD i = 0; i < bytesRead; ++i)
        {
            char c = buffer[i];
            if (c == '\r') continue;
            if (c == '\n')
            {
                if (!word.empty())
                {
                    files.push_back(std::move(word));
                    word.clear();
                }
                continue;
            }
            word.push_back(c);
        }
    }
    if (!word.empty())
        files.push_back(std::move(word));


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
void recursiveHTMLFileSystemCreator(const fs::path& path, std::string& out, int depth)
 {

     out += "<ul class=\"tree\">\n";

    std::error_code ec;

    fs::directory_iterator it(path, fs::directory_options::skip_permission_denied, ec);
    if (ec) {
        std::cerr << "Error abriendo " << path << ": " << ec.message() << "\n";
        out += + "</ul>\n";
        return;
    }
    for (const auto& entry : it)
    {
        try
        {
            auto fileRelativePath = entry.path();
            const std::string name = htmlEscape(fileRelativePath.filename().string());
            auto relativePath = htmlEscape(entry.path().string());
            if (entry.is_regular_file())
            {
                std::string extension = entry.path().extension().string();

                std::transform(
                    extension.begin(),
                    extension.end(),
                    extension.begin(),
                    [](unsigned char c) { return std::tolower(c); }
                );

                out += "<li><a href=\"" + relativePath + "\" download><svg class=\"icon\"><use href=\"#" + extension +"\"></use></svg>" + name +"</a></li>";

            } else
            {
                out += "<li class=\"folder\"><details><summary>" + name + "<a href=\"" + relativePath + "\" download class=\"desc\">Download as zip</a></summary>";
                if (depth +1 < maxHTMLRecursionDepth){
                    recursiveHTMLFileSystemCreator(fileRelativePath,out,depth+1);
                }

                out += "</details></li>\n";
            }
        }catch (const fs::filesystem_error& e) {
            std::cerr << "Error with path: " << entry.path() << " : " << e.what() << std::endl;
            continue;                   // sigue con el siguiente archivo
        }catch (const std::exception& e)
        {
            std::cerr << "Error inesperado: " << e.what() << "\n";
            continue;
        }

    }

     out+= "</ul>\n";

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

#include "zip.h"
#include <cstdlib>
#include <iostream>

namespace fs = std::filesystem;

// Comprime una carpeta y devuelve los bytes del .zip (vacío si falla)
std::vector<unsigned char> compressDirectory(const fs::path& directory) {
    std::vector<unsigned char> ressult;

    if (!fs::is_directory(directory)) {
        std::cerr << "La carpeta no existe: " << directory << "\n";
        return ressult;
    }

    zip_t* zip = zip_stream_open(nullptr, 0, ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');
    if (!zip) {
        std::cerr << "No se pudo crear el zip en memoria\n";
        return ressult;
    }

    bool ok = true;
    for (const auto& e : fs::recursive_directory_iterator(directory)) {
        if (!e.is_regular_file()) continue;

        // Nombre dentro del zip: ruta relativa con '/' como separador
        std::string nombre = fs::relative(e.path(), directory).generic_string();
        std::string real   = e.path().string();

        if (zip_entry_open(zip, nombre.c_str()) != 0) { ok = false; break; }
        if (zip_entry_fwrite(zip, real.c_str()) != 0) {
            zip_entry_close(zip);
            ok = false;
            break;
        }
        zip_entry_close(zip);
    }

    if (ok) {
        char* buf = nullptr;
        size_t bufsize = 0;
        if (zip_stream_copy(zip, (void**)&buf, &bufsize) >= 0 && buf) {
            ressult.assign(reinterpret_cast<unsigned char*>(buf),
                             reinterpret_cast<unsigned char*>(buf) + bufsize);
        }
        std::free(buf);
    }

    zip_stream_close(zip);
    return ressult;
}


