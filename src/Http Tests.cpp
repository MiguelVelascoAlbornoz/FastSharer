

#include <ws2tcpip.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <thread>
#include <windows.h>
#include <wininet.h> // Adicione esta linha
#include <mutex>
#include "../build/generated/resources.h"
#include "dirGetter.h"

#include <atomic>

#include "Commands.h"

std::atomic<bool> running{true};


static WSADATA wsaData;


static std::string get_local_ip() {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR)
        return "localhost";

    addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfo* result = nullptr;
    if (getaddrinfo(hostname, nullptr, &hints, &result) != 0 || !result)
        return "localhost";

    sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(result->ai_addr);
    //std::string ip = inet_ntoa(addr->sin_addr);
    // Substitua esta linha:
    // std::string ip = inet_ntoa(addr->sin_addr);

    // Por este bloco:
    char ipStr[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &(addr->sin_addr), ipStr, INET_ADDRSTRLEN) == nullptr) {
        freeaddrinfo(result);
        return "localhost";
    }
    std::string ip = ipStr;
    freeaddrinfo(result);
    return ip;
}

static std::string get_public_ip() {
    HINTERNET hInternet = InternetOpenA("IP retriever", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet) return "N/A";
    // Usa um serviço que retorna apenas IPv4
    HINTERNET hFile = InternetOpenUrlA(hInternet, "http://ipv4.icanhazip.com/", NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hFile) {
        InternetCloseHandle(hInternet);
        return "N/A";
    }
    char buffer[128] = { 0 };
    DWORD bytesRead = 0;
    std::string ip;
    if (InternetReadFile(hFile, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        ip = buffer;
        // Remove espaços e quebras de linha
        ip.erase(std::remove(ip.begin(), ip.end(), '\n'), ip.end());
        ip.erase(std::remove(ip.begin(), ip.end(), '\r'), ip.end());
    }
    else {
        ip = "N/A";
    }
    InternetCloseHandle(hFile);
    InternetCloseHandle(hInternet);
    return ip;
}


static std::string get_content_type(const std::string& path) {
    if (path.find(".html") != std::string::npos) return "text/html";
    if (path.find(".txt") != std::string::npos) return "text/plain";
    if (path.find(".xml") != std::string::npos) return "application/xml";
    return "application/octet-stream";
}

#include <filesystem>



namespace fs = std::filesystem;

bool resolveSafe(const fs::path& root, const std::string& urlPath, fs::path& out)
{
    std::string p = url_decode(urlPath);              // una sola vez
    if (p.empty() || p[0] != '/') return false;
    if (p.find('\0') != std::string::npos) return false;
    if (p.find('\\') != std::string::npos) return false; // opcional, pero simplifica
    if (p == "/") return false;

    std::error_code ec;
    fs::path rootCanon = fs::weakly_canonical(root, ec);
    if (ec) return false;
    fs::path full = fs::weakly_canonical(rootCanon / p.substr(1), ec);
    if (ec) return false;

    // full debe estar dentro de rootCanon
    auto [rEnd, fIt] = std::mismatch(rootCanon.begin(), rootCanon.end(),
                                     full.begin(), full.end());
    if (rEnd != rootCanon.end()) return false;

    out = full;
    return true;
}
static void TCPThread(SOCKET serverSocket, std::string baseDir)
{

 while (true) {
      fd_set readSet;
      FD_ZERO(&readSet);
      FD_SET(serverSocket, &readSet);
      timeval tv{1, 0}; // 1 segundo

      int r = select(0, &readSet, nullptr, nullptr, &tv);
      if (r == SOCKET_ERROR) break;
      if (r == 0) continue; // timeout, vuelve a comprobar running
     sockaddr_in clientAddr;
     int addrSize = sizeof(clientAddr);
     SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &addrSize);
     DWORD timeoutMs = 5000;
     setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO,
                reinterpret_cast<const char*>(&timeoutMs), sizeof(timeoutMs));

        if (!running) break;
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
        std::cout << "Conexion decode: " << clientIP << "\n";

        if (clientSocket == INVALID_SOCKET) {
            if (!running) break;
            std::cerr << "Error en accept\n";
            continue; // o cerrar servidor segun convenga
        }
        char buffer[1024];
		int bytes = recv(clientSocket, buffer, sizeof(buffer), 0); //lee la solicitud del cliente y la guarda en buffer
        if (bytes > 0) {
			std::istringstream request(buffer);
            std::string method, path, protocol;
            std::cout << request.str() << "\n";
            request >> method >> path >> protocol; //escrite la primera segunda y tercera palavra en method path y protocol respectivamente

            //std::string fullPath = baseDir + path;
            //std::replace(fullPath.begin(), fullPath.end(), '/', '\\');

            //char absolutePath[MAX_PATH];
            //DWORD length = GetFullPathNameA(fullPath.c_str(), MAX_PATH, absolutePath, nullptr);

            //std::string absPathStr = absolutePath;
            // Bloquear rutas que salgan del directorio base
            //if (absPathStr.find(baseDir) != 0) {
            //    // Si intenta salir del directorio base, servir 404
            //    absPathStr = baseDir + "\\404.html"; // o puedes crear contenido "<h1>403 Forbidden</h1>"

            //}
            std::filesystem::path outPath;
            resolveSafe(baseDir,path, outPath);
            std::ifstream file(outPath, std::ios::binary);
            std::ostringstream response;
			//generamos la respuesta HTTP en caso de existir el direcotrio o no
            if (file) {
                std::ostringstream ss;
                ss << file.rdbuf();
                std::string content = ss.str();

                response << "HTTP/1.1 200 OK\r\n";
                response << "Content-Type: " << get_content_type(outPath.string()) << "\r\n";
                response << "Content-Length: " << content.size() << "\r\n";
                response << "\r\n";
                response << content;
                std::cout << "Valid request...\n\n\n\n\n";
            }
            else {
                std::string htmlFiles = recursiveHTMLFileSystemCreator("");

                std::string html(reinterpret_cast<const char*>(INDEX_DATA), INDEX_SIZE);
                replaceAll(html, "{{FILES}}", htmlFiles);

                std::string content = "<h1>404 Not Found</h1>";
                response << "HTTP/1.1 404 Not Found\r\n";
                response << "Content-Type: text/html\r\n";
                response << "Content-Length: " << html.size() << "\r\n";
                response << "\r\n";
                response << (html);
                std::cout << "Invalid request, opening index...\n\n\n\n\n";;
            }

            std::string res_str = response.str();
            send(clientSocket, res_str.c_str(), res_str.size(), 0);
        }

        closesocket(clientSocket);
    }
}


int main()
{

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error al inicializar Winsock\n";
        return 1;
    }
	//discover_upnp();
    //Crea un socket TCP (flujo de bytes) para IPv4 (AF_INET).
    //SOCK_STREAM indica TCP(no UDP).
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error al crear socket\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET; //tipo de conexion (IPv4)
    serverAddr.sin_addr.s_addr = INADDR_ANY;//acepta conexiones desde cualquier IP de tu PC
    serverAddr.sin_port = htons(8080); //puerto del servidor (8080). htons() convierte a orden de bytes de red.

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error en bind\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        std::cerr << "Error en listen\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    std::cout << "Servidor HTTP escuchando en puerto 8080...\n";
    std::cout << "Server listening in : http://" << get_local_ip() << ":8080/" << std::endl;
    std::cout << "Server listening in : http://" << get_public_ip() << ":8080/" << std::endl;
    // Carpeta segura
    char exePath[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, exePath);

    // Busca a ultima barra invertida e termina a cadeia de caracteres
    //char* lastSlash = strrchr(exePath, '\\');
    //if (lastSlash) {
    //  *lastSlash = '\0'; // Agora exePath contem apenas o diretorio
    //}

    std::string baseDir = exePath;

    std::thread tcpThread(TCPThread,serverSocket,exePath);


    Commands::startCMD(exePath);
    running = false;
    closesocket(serverSocket);

    tcpThread.join();


    WSACleanup();
    return 0;
}

