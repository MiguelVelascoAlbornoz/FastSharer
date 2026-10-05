//
// Created by migue on 05/10/2026.
//


#ifndef FASTSHARER_DIRGETTER_H
#define FASTSHARER_DIRGETTER_H
#include <vector>
#include <filesystem>
extern  int maxHTMLRecursionDepth;

bool getFiles(const char* path, std::vector<std::string>& files);
void recursiveHTMLFileSystemCreator(const std::filesystem::path& path, std::string& out, int depth);
void replaceAll(std::string& text, const std::string& key, const std::string& value);
std::string url_decode(const std::string& value);
std::vector<unsigned char> compressDirectory(const std::filesystem::path& directory) ;
#endif //FASTSHARER_DIRGETTER_H
