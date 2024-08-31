#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <regex>
#include <sstream>


using namespace std;

class Linker {
public:
    void link(const std::string& objFile1, const std::string& objFile2, const std::string& outputFile);
    
private:
    void parseOBJFile(const std::string& filePath, std::unordered_map<std::string, int>& symbolTable,
                      std::unordered_map<std::string, int>& definitionTable,
                      std::unordered_map<std::string, int>& usageTable, std::vector<int>& code,
                      std::string& relocationTable, int& moduleSize);
    void resolveReferences(std::unordered_map<std::string, int>& globalSymbolTable, 
                           std::unordered_map<std::string, int>& usageTable,
                           std::unordered_map<std::string, int>& definitionTable,
                           std::vector<int>& code);
};

void Linker::link(const std::string& objFile1, const std::string& objFile2, const std::string& outputFile) {
    std::unordered_map<std::string, int> globalSymbolTable;
    std::unordered_map<std::string, int> usageTable1, usageTable2, definitionTable1, definitionTable2;
    std::vector<int> code1, code2;
    std::string relocationTable1, relocationTable2;
    int moduleSize1 = 0, moduleSize2 = 0;

    // Parse the OBJ files and extract relevant data
    parseOBJFile(objFile1, globalSymbolTable, usageTable1, definitionTable1, code1, relocationTable1, moduleSize1);
    // Correction factors
    int correctionFactor2 = moduleSize1;
    parseOBJFile(objFile2, globalSymbolTable, usageTable2, definitionTable2, code2, relocationTable2, correctionFactor2);

    cout << "Module Size 1: " << moduleSize1 << endl;
    cout << "Module Size 2: " << correctionFactor2 << endl;

    cout << "Definition Table 1: " << endl;
    for (auto& [symbol, address] : definitionTable1) {
        cout << symbol << " " << address << endl;
    }
    cout << endl;

    cout << "Definition Table 2: " << endl;
    for (auto& [symbol, address] : definitionTable2) {
        cout << symbol << " " << address << endl;
    }
    cout << endl;

    cout << "Global Symbol Table: " << endl;
    for (auto& [symbol, address] : globalSymbolTable) {
        cout << symbol << " " << address << endl;
    }

    cout << "Usage Table 1: " << endl;
    for (auto& [symbol, pos] : usageTable1) {
        cout << symbol << " " << pos << endl;
    }

    cout << "Usage Table 2: " << endl;
    for (auto& [symbol, pos] : usageTable2) {
        cout << symbol << " " << pos << endl;
    }

    cout << "Code 1: " << endl;
    for (auto& code : code1) {
        cout << code << " ";
    }
    cout << endl;

    cout << "Code 2: " << endl;
    for (auto& code : code2) {
        cout << code << " ";
    }
    cout << endl;

    // Resolve references and correct addresses for each object
    resolveReferences(globalSymbolTable, usageTable1, definitionTable2 , code1);
    resolveReferences(globalSymbolTable, usageTable2, definitionTable1, code2);


    // Write the linked output to a file
    std::ofstream output(outputFile);
    if (!output) {
        throw std::runtime_error("Could not open output file: " + outputFile);
    }

    for (const auto& code : code1) {
        output << code << " ";
    }
    for (const auto& code : code2) {
        output << code << " ";
    }
    output.close();
}

void Linker::parseOBJFile(const std::string& filePath, std::unordered_map<std::string, int>& symbolTable,
                          std::unordered_map<std::string, int>& usageTable,
                          std::unordered_map<std::string, int>& definitionTable, std::vector<int>& code,
                          std::string& relocationTable, int& moduleSize)
{
    std::ifstream input(filePath);
    if (!input) {
        throw std::runtime_error("Could not open input file: " + filePath);
    }

    std::string line;
    bool inDefinitionTable = false;
    bool inUsageTable = false;
    bool firstUsageLine = true;

    while (std::getline(input, line)) {
        if (firstUsageLine) {
                std::istringstream iss(line);
                int value;
                while (iss >> value) {
                    code.push_back(value);
                }
                firstUsageLine = false;
                continue;
        }
        if (line.find("DEFINITION TABLE:") != std::string::npos) {
            inDefinitionTable = true;
            inUsageTable = false;
            continue;
        }
        if (line.find("USAGE TABLE:") != std::string::npos) {
            inDefinitionTable = false;
            inUsageTable = true;
            continue;
        }
        if (inDefinitionTable) {
            std::istringstream iss(line);
            std::string symbol;
            int address;
            while (iss >> symbol >> address) {
                symbolTable[symbol] = address + moduleSize;
                definitionTable[symbol] = address + moduleSize;
            }
            continue;
        }
        if (inUsageTable) {
            std::istringstream iss(line);
            std::string symbol;
            int pos;
            while (iss >> symbol >> pos) {
                usageTable[symbol] = pos;
            }
            continue;
        }
    }
    moduleSize = code.size();
    
    input.close();
}

void Linker::resolveReferences(std::unordered_map<std::string, int>& globalSymbolTable, std::unordered_map<std::string, int>& usageTable, std::unordered_map<std::string, int>& definitionTable,
                               std::vector<int>& code) {
    for (const auto& [symbol, pos] : usageTable) {
        if (globalSymbolTable.find(symbol) != globalSymbolTable.end()) {
            int symbolAddress = globalSymbolTable[symbol];
            code[pos] = symbolAddress;
        } else {
            throw std::runtime_error("Undefined symbol: " + symbol);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <prog1.obj> <prog2.obj>" << std::endl;
        return 1;
    }

    std::string objFile1 = argv[1];
    std::string objFile2 = argv[2];
    std::string outputFile = objFile1.substr(0, objFile1.find_last_of('.')) + ".e";

    Linker linker;
    try {
        linker.link(objFile1, objFile2, outputFile);
        std::cout << "Linked output written to " << outputFile << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}