//
// Created by migue on 04/10/2026.
//

#ifndef FASTSHARER_COMMANDS_H
#define FASTSHARER_COMMANDS_H
#define MAX_USER_INPUT 1024
#include <vector>
#include <vector>
#include <bits/basic_string.h>

class Commands
{
public:
    static void setDepth(std::vector<std::string>& args);
    static void breakCommand(const char* userInput, std::vector<std::string>& args);
    static void helpCMD();
    static void ls();

    static void startCMD(char* path);


};


#endif //FASTSHARER_COMMANDS_H
