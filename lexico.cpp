#include <iostream>
#include <cstring>
#include <vector>
#include "lexico.hpp"

std::vector<std::string> intrucoes = {"ADD","SUB","MOV","COPY"};

void analise_lexa(std::string& linha)
{
    std::vector<std::string> tokens;
    char *plinha = std::strtok(&linha[0], " ,:");
    while(plinha != NULL)
    {
        std::cout << plinha <<std::endl;
        tokens.push_back(plinha);
        plinha = std::strtok(NULL, " ,:");
    }



    //std::cout << "Funfa!" << std::endl;
}
