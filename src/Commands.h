//
// Created by migue on 04/10/2026.
//

#ifndef FASTSHARER_COMMANDS_H
#define FASTSHARER_COMMANDS_H
#define MAX_USER_INPUT 1024
#include <vector>

class Commands
{
public:

    static void helpCMD();
    static void ls();

    static void startCMD(char* path);


};


#endif //FASTSHARER_COMMANDS_H
