
#pragma comment(lib, "Ws2_32.lib")
#include <ws2tcpip.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <windows.h>
#include <wininet.h> // Adicione esta linha
#pragma comment(lib, "Wininet.lib") // Adicione e
WSADATA wsaData;


std::string get_local_ip() {
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
std::string get_public_ip() {
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
std::string url_decode(const std::string& value) {
    std::string result;
    char ch;
    int i, ii;
    for (i = 0; i < value.length(); i++) {
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

std::string get_content_type(const std::string& path) {
    if (path.find(".html") != std::string::npos) return "text/html";
    if (path.find(".txt") != std::string::npos) return "text/plain";
    if (path.find(".xml") != std::string::npos) return "application/xml";
    return "application/octet-stream";
}
int main()
{
    //inicializa librerias

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

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET; //tipo de conexion (IPv4)
    serverAddr.sin_addr.s_addr = INADDR_ANY;//acepta conexiones desde cualquier IP de tu PC
    serverAddr.sin_port = htons(8080); //puerto del servidor (8080). htons() convierte a orden de bytes de red.

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
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
;  // Carpeta segura
char exePath[MAX_PATH];
GetModuleFileNameA(NULL, exePath, MAX_PATH);

// Busca a última barra invertida e termina a cadeia aí
char* lastSlash = strrchr(exePath, '\\');
if (lastSlash) {
    *lastSlash = '\0'; // Agora exePath contém apenas o diretório
}

std::string baseDir = exePath;
    while (true) {
		//para el programa haste que llega una conexion y se la asigna al socket
        sockaddr_in clientAddr;
        int addrSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &addrSize);

        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
        std::cout << "Conexion desde: " << clientIP << "\n";

        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Error en accept\n";
            continue; // o cerrar servidor según convenga
        }
        char buffer[1024];
		int bytes = recv(clientSocket, buffer, sizeof(buffer), 0); //lee la solicitud del cliente y la guarda en buffer
        if (bytes > 0) {
			std::istringstream request(buffer);
            std::string method, path, protocol;
            std::cout << request.str() << "\n";
            request >> method >> path >> protocol; //escrite la primera segunda y tercera palavra en method path y protocol respectivamente
            path = url_decode(path);
            if (path == "/") path = "/index.html";
            std::string fullPath = baseDir + path;
            std::replace(fullPath.begin(), fullPath.end(), '/', '\\');

            char absolutePath[MAX_PATH];
            DWORD length = GetFullPathNameA(fullPath.c_str(), MAX_PATH, absolutePath, nullptr);
            
            std::string absPathStr = absolutePath;
            // Bloquear rutas que salgan del directorio base
            if (absPathStr.find(baseDir) != 0) {
                // Si intenta salir del directorio base, servir 404
                absPathStr = baseDir + "\\404.html"; // o puedes crear contenido "<h1>403 Forbidden</h1>"

            }
            
            std::ifstream file(absPathStr, std::ios::binary);
            std::ostringstream response;
			//generamos la respuesta HTTP en caso de existir el direcotrio o no
            if (file) {
                std::ostringstream ss;
                ss << file.rdbuf();
                std::string content = ss.str();

                response << "HTTP/1.1 200 OK\r\n";
                response << "Content-Type: " << get_content_type(absPathStr) << "\r\n";
                response << "Content-Length: " << content.size() << "\r\n";
                response << "\r\n";
                response << content;
                std::cout << "Valid request...\n\n\n\n\n";
            }
            else {
                std::string content = "<h1>404 Not Found</h1>";
                response << "HTTP/1.1 404 Not Found\r\n";
                response << "Content-Type: text/html\r\n";
                response << "Content-Length: " << content.size() << "\r\n";
                response << "\r\n";
                response << content;
                std::cout << "Invalid request...\n\n\n\n\n";
            }

            std::string res_str = response.str();
            send(clientSocket, res_str.c_str(), res_str.size(), 0);
        }

        closesocket(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}

