#include <string>

std::string aggregateSearchMonitor(char* A, char* B, char* C, std::string& def, std::string& similar,bool& ASim, bool& BSim,bool& CSim);
std::string aggregateSearchClient(char* A, char* B, char* C);
std::string aggregatePrefixSuffixClient(char* A, char* B, char* C);
std::string aggregatePrefixSuffixMonitor(char* A, char* B, char* C,int& cA,int& cB, int& cC);
