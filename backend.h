#pragma once

#include <string>
#include <unordered_map>
#include <vector>

typedef struct prefixTreeNode{
	prefixTreeNode* children[30];
	bool end;
}prefixTreeNode;

class prefixTree{
private:
	prefixTreeNode* head;
public:
	prefixTree();
	void insert(std::string input);
	bool search(std::string input);
	std::vector <std::string> startsWith(std::string  input);
};

typedef struct suffixTreeNode{
	suffixTreeNode* children[30];
	bool start;
}suffixTreeNode;

class suffixTree{
	private:
	suffixTreeNode* head;
	public:
	suffixTree();
	void insert(std::string input);
	bool search(std::string input);
	std::vector <std::string> endsWith(std::string  input);
};


//Loads data structures using file with name dictName
void startup(std::string dictName, std::unordered_map <std::string,std::string>& dictionary, prefixTree& prefix);
//Output format:
	//DIDN'T FIND ANYTHING: 0&INPUT
	//SEARCH AND FOUND SIMILAR: 1&INPUT&VALUE&WORD&DEFINITION
	//SEARCH AND DIDN'T FIND SIMILAR: 1&INPUT&VALUE
	//DIDNT FIND BUT FOUND SIMILAR: 1&&INPUT&WORD&DEFINITION
	//PREFIX/SUFFIX: (2|3)&n&INPUT&VALUE1&VALUE2&VALUE3...&VALUEn
char* search(char* input, std::unordered_map <std::string,std::string>& dict, char server);
char* prefix(char* input, prefixTree& prefix, char server);
char* suffix(char* input, suffixTree& suffix, char server);