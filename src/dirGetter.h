//
// Created by migue on 05/10/2026.
//


#ifndef FASTSHARER_DIRGETTER_H
#define FASTSHARER_DIRGETTER_H
#include <vector>
 bool getFiles(const char* path, std::vector<std::string>& files);
  std::string recursiveHTMLFileSystemCreator(const char* path);
void replaceAll(std::string& text, const std::string& key, const std::string& value);
std::string url_decode(const std::string& value);
#endif //FASTSHARER_DIRGETTER_H
