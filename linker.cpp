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
                      std::string& relocationTable, int& moduleSize, std::vector<int>& realCode);
    void resolveReferencesMod1(std::unordered_map<std::string, int>& globalSymbolTable, 
                           std::unordered_map<std::string, int>& usageTable,
                           std::unordered_map<std::string, int>& definitionTable,
                           std::vector<int>& code, int& fatorCorrecao);
    void resolveReferencesMod2(std::unordered_map<std::string, int>& globalSymbolTable, 
                           std::unordered_map<std::string, int>& usageTable,
                           std::unordered_map<std::string, int>& definitionTable,
                           std::vector<int>& code, std::vector<int>& realCode, int& fatorCorrecao);
};

void Linker::link(const std::string& objFile1, const std::string& objFile2, const std::string& outputFile) {
    std::unordered_map<std::string, int> globalSymbolTable;
    std::unordered_map<std::string, int> usageTable1, usageTable2, definitionTable1, definitionTable2;
    std::vector<int> code1, code2, realCode1, realCode2;
    std::string relocationTable1, relocationTable2;
    int moduleSize1 = 0, moduleSize2 = 0;

    // Parse the OBJ files and extract relevant data
    parseOBJFile(objFile1, globalSymbolTable, usageTable1, definitionTable1, code1, relocationTable1, moduleSize1, realCode1);
    // Correction factors
    int correctionFactor2 = moduleSize1;
    parseOBJFile(objFile2, globalSymbolTable, usageTable2, definitionTable2, code2, relocationTable2, correctionFactor2, realCode2);

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

    cout << "Real Code 1: " << endl;
    for(auto& real : realCode1){
        cout << real << " ";
    }

    cout << "Real Code 2: " << endl;
    for(auto& real : realCode2){
        cout << real << " ";
    }
    // Resolve references and correct addresses for each object
    resolveReferencesMod1(globalSymbolTable, usageTable1, definitionTable2, code1,  moduleSize1);
    resolveReferencesMod2(globalSymbolTable, usageTable2, definitionTable2, code2, realCode2,  moduleSize1);


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
                          std::string& relocationTable, int& moduleSize, std::vector<int>& realCode)
{
    std::ifstream input(filePath);
    if (!input) {
        throw std::runtime_error("Could not open input file: " + filePath);
    }

    std::string line;
    bool inDefinitionTable = false;
    bool inUsageTable = false;
    bool firstUsageLine = true;
    bool inReal = false;

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
            inReal = false;
            continue;
        }
        if (line.find("USAGE TABLE:") != std::string::npos) {
            inDefinitionTable = false;
            inUsageTable = true;
            inReal = false;
            continue;
        }
        if(line.find("REAL:") != std::string::npos){
            inReal = true;
            inUsageTable = false;
            inDefinitionTable = false;
            continue;
        }
        if (inDefinitionTable) {
            std::istringstream iss(line);
            std::string symbol;
            int address;
            while (iss >> symbol >> address) {
                symbolTable[symbol] = address + moduleSize;
                definitionTable[symbol] = address;
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
        if(inReal){
            std::istringstream iss(line);
            for (size_t i = 0; i < line.size(); ++i) {
                realCode.push_back(static_cast<int>(line[i] - '0'));
            }
            break;
        }
    }
    moduleSize = code.size();
    
    input.close();
}

void Linker::resolveReferencesMod1(std::unordered_map<std::string, int>& globalSymbolTable, std::unordered_map<std::string, int>& usageTable, std::unordered_map<std::string, int>& definitionTable,
                               std::vector<int>& code, int& fatorCorrecao) {
    for (const auto& [symbol, pos] : usageTable) {
        if (globalSymbolTable.find(symbol) != globalSymbolTable.end()) {
            int symbolAddress = globalSymbolTable[symbol];
            code[pos] += symbolAddress;
        } else {
            throw std::runtime_error("Undefined symbol: " + symbol);
        }
    }                       
}
void Linker::resolveReferencesMod2(std::unordered_map<std::string, int>& globalSymbolTable, std::unordered_map<std::string, int>& usageTable, std::unordered_map<std::string, int>& definitionTable,
                               std::vector<int>& code, std::vector<int>& realCode, int& fatorCorrecao) {
    
    std::vector<int> posUsageTable;
    for(auto [symbol, pos] : usageTable) {
        if (globalSymbolTable.find(symbol) != globalSymbolTable.end()) {
            int symbolAddress = globalSymbolTable[symbol];
            code[pos] += symbolAddress;
            posUsageTable.push_back(pos);
        } else {
            throw std::runtime_error("Undefined symbol: " + symbol);
        }
    }
    
    cout << "posUsageTable: " << endl;
    for (auto& code : posUsageTable) {
        cout << code << " ";
    }
    cout << endl;


    if(code.size() == realCode.size()){
        for(size_t i = 0; i < code.size(); ++i){
            if(realCode[i] == 1) {
                if(std::find(posUsageTable.begin(), posUsageTable.end(), i) == posUsageTable.end()) {
                    code[i] += fatorCorrecao;
                }
            }
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