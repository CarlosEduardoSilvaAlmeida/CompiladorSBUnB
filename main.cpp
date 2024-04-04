#include <iostream>
#include <fstream>
#include <string>
#include "lexico.hpp"


using namespace std;


void analise_lexa(std::string& linha);


int main()
{
    std::ifstream file("codigo.txt");
    if(!file)
        cout << "Falhou";
    std::string linha;
    while (std::  getline(file, linha))
    {
        //std::cout << linha << '\n';
        analise_lexa(linha);
    }

    file.close(); // Fecha o arquivo
    return 0;
}
