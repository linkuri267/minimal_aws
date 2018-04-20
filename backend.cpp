#include "backend.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

int getAlphIndex(char c){
	int alphIndex;
	if(c == '-'){ // dash
		alphIndex = 26;
	}
	else if(c == ' '){ //space 
		alphIndex = 27;	
	}
	else if((int)c == 0x27){ //' character'
		alphIndex = 28;
	}
	else if(c == '.'){
		alphIndex = 29;
	}
	else if(((int)c >= 0x41)&&((int)c <= 0x5A)){ //capital
		alphIndex = (int)c - 0x40;
	}
	else{
		alphIndex = (int)c - 0x60;
	}
	return(alphIndex);
}

char getCharFromIndex(int index){
	char c;
	if(index == 26){
		c = (char)0x2D;
	}
	else if(index == 27){
		c = (char)0x20;
	}
	else if(index == 28){
		c = (char)0x27;
	}
	else if(index == 29){
		c = '.';
	}
	else{
		c = (char)(index + 0x60);
	}
	return(c);
}


prefixTree::prefixTree(){
	this->head = new prefixTreeNode;
}

void prefixTree::insert(std::string input){
	prefixTreeNode* it = this->head;
	int i = 0;
	int alphIndex;
	while(i < input.size()){
		alphIndex = getAlphIndex(input[i]);
		if(it->children[alphIndex] == NULL){
			it->children[alphIndex] = new prefixTreeNode(); 
		}
		it = it->children[alphIndex];
		i++;
	}
	it->end = true;
}

bool prefixTree::search(std::string input){
	int i = 0;
	prefixTreeNode* it = this->head;
	int alphIndex;

	while(i < input.size()){
		alphIndex = getAlphIndex(input[i]);
		if(it->children[alphIndex] != NULL){
			it = it->children[alphIndex];
		}
		else{
			return(false);
		}
		i++;
	}
	if(it->end){
		return(true);
	}
	else{
		return(false);
	}
}

void startsWithHelper(prefixTreeNode* it, std::string current, std::vector <std::string>& words){
	if(it->end){
		words.push_back(current);
	}	
	for(int i = 0; i < 30; i++){
		if(it->children[i] != NULL){
			startsWithHelper(it->children[i],current + getCharFromIndex(i), words);
		}
	}
	return;
}


std::vector <std::string> prefixTree::startsWith(std::string input){
	std::vector <std::string> words (0);
	prefixTreeNode* it = this->head;
	int alphIndex;

	//Get to node 
	for(int i = 0; i < input.size(); i++){
		alphIndex = getAlphIndex(input[i]);
		if(it->children[alphIndex] == NULL){
			return(words);
		}
		it = it->children[alphIndex];
	}
	std::string current = input;
	if(it->end){
		words.push_back(input);
	}
	startsWithHelper(it,current,words);
	return(words);
}

suffixTree::suffixTree(){
	this->head = new suffixTreeNode;
}


void suffixTree::insert(std::string input){
	suffixTreeNode* it = this->head;
	int alphIndex;
	int i = input.size() -1;
	while(i >= 0){
		alphIndex = getAlphIndex(input[i]);
		if(it->children[alphIndex] == NULL){
			it->children[alphIndex] = new suffixTreeNode();

		}
		it = it->children[alphIndex];
		i--;
	}
	it->start = true;
}

bool suffixTree::search(std::string input){
	suffixTreeNode* it = this->head;
	int alphIndex;
	for(int i = input.size() - 1; i >= 0; i --){
		alphIndex = getAlphIndex(input[i]);
		if(it->children[alphIndex] == NULL){
			return(false);
		}
		it = it->children[alphIndex];
	}		
	if(it->start){
		return(true);
	} 
	else{
		return(false);
	}
}


void endsWithHelper(std::vector <std::string>& values, suffixTreeNode* it, std::string current){
	if(it->start){
		values.push_back(current);
	}
	for(int i = 0; i < 30; i++){
		if(it->children[i] != NULL){
			endsWithHelper(values,it->children[i],getCharFromIndex(i)+ current);
		}
	}
	return;
}

std::vector <std::string> suffixTree::endsWith(std::string input){
	suffixTreeNode* it = this->head;
	int alphIndex;
	std::vector <std::string> values (0);

	//get to the furthest node
	for(int i = input.size() -1 ; i >= 0; i --){
		alphIndex = getAlphIndex(input[i]);
		if(it->children[alphIndex] == NULL){
			return values;
		}
		it = it->children[alphIndex];
	}
	if(it->start){
		values.push_back(input);
	}
	endsWithHelper(values,it,input);
	return(values);

}



void startup(std::string dictName, std::unordered_map <std::string,std::string>& dictionary, prefixTree& prefix, suffixTree& suffix){
	std::ifstream dictIn;
	dictIn.open(dictName);
	std::string tempLine;
	std::string tempWord;
	std::string tempDef;
	int count = 0;

	while(std::getline(dictIn,tempLine)){
		//std::cout << "Line: " << tempLine << std::endl;
		int i = 0;
		while(tempLine[i] != ':'){
			i++;
		}
		tempWord = tempLine.substr(0,i-1);
		i += 3;
		tempDef = tempLine.substr(i);
		dictionary.insert({tempWord,tempDef});
		//std::cout << "Word:" << tempWord << " Definition:" << tempDef << std::endl;
		prefix.insert(tempWord);
		suffix.insert(tempWord);
	}

}

//returns one similar word
//returns empty string if none found
std::string findSimilar(std::string word, std::unordered_map <std::string, std::string>& dict){
	std::string temp = "";
	char specialCharacters[] = {' ','-',(char)0x27,'.'};
	std::vector <std::string> similars;
 
	for(int i = 0; i < word.length(); i++){
		temp = word;
		//lower case
		for(int l = 0; l < 26; l++){
			temp[i] = (char)(l + 0x60);
			if((dict.find(temp) != dict.end())&&(temp != word)){
				similars.push_back(temp);
			}
		}
		//capital
		for(int l = 0; l < 26; l++){
			temp[i] = (char)(l + 0x40);
			if((dict.find(temp) != dict.end())&&(temp != word)){
				similars.push_back(temp);
			}
		}
		//special characters
		for(int l = 0; l < 4; l++){
			temp[i] = specialCharacters[l];
			if((dict.find(temp) != dict.end())&&(temp != word)){
				similars.push_back(temp);
			}
		}
	}
	std::cout << "and <" << similars.size() << "> similar words" << std::endl;
	if(!similars.empty())
	{
		temp = similars[0];
	}
	else{
		temp = "";
	}
	return(temp);
} 

char* search(char* input, std::unordered_map <std::string,std::string>& dict, char server){
	std::string definition;
	std::string inputWord;
	std::string similar;
	std::string similarDef;
	bool foundSimilar = false;
	bool found = false;

	int i = 2;
	while(input[i] != '\0'){
		inputWord += input[i];
		i++;
	}
	std::cout << "The server" << server << " received input <" << inputWord << "> and operation <search>" << std::endl;
	std::cout << "The server" << server <<" has found <";
	if(dict.find(inputWord) != dict.end()){
		found = true;
		definition= dict[inputWord];
		std::cout << "1> match ";
	}
	else{
		std::cout << "0> match ";
	}

	similar = findSimilar(inputWord,dict);
	if(similar != ""){
		foundSimilar = true;
		similarDef = dict[similar];
	}


	// std::cout << "Input Word:" << inputWord << std::endl;
	// std::cout << "Input Def:" << definition << std::endl;
	// std::cout << "Similar:" << similar << std::endl;
	// std::cout << "Similar Def:" << similarDef << std::endl;

	//format message
	std::string out;
	int size;
	char* out_c;

	if(found){
		if(foundSimilar){
			//1&INPUT&VALUE&WORD&DEFINITION
			out += '1';
			out += '&';
			out.append(inputWord);
			out += '&';
			out.append(definition);
			out += '&';
			out.append(similar);
			out += '&';
			out.append(similarDef);
		}
		else{
			//1&INPUT&VALUE
			out += '1';
			out += '&';
			out.append(inputWord);
			out += '&';
			out.append(definition);
		}
	}
	else{
		if(foundSimilar){
			//1&&INPUT&WORD&DEFINITION
			out += '1';
			out += '&';
			out += '&';
			out.append(inputWord);
			out += '&';
			out.append(similar);
			out += '&';
			out.append(similarDef);	
		}
		else{
			//0&INPUT
			out += '0';
			out += '&';
			out.append(inputWord);
		}
	}
	out_c = new char[out.length()+1];
	strcpy(out_c,out.c_str());
	return(out_c);


}	

char* prefix(char* input, prefixTree& prefix, char server){
	std::string inputWord;
	std::string out;
	char* out_c;
	int i = 2;
	while(input[i] != '\0'){
		inputWord += input[i];
		i++;
	}
	std::cout << "The server" << server << " received input <" << inputWord << "> and operation <prefix>" << std::endl;
	std::vector <std::string> prefixes = prefix.startsWith(inputWord);
	std::cout << "The Server" << server << " has found <" << prefixes.size() << "> matches" << std::endl; 
	// for(int i = 0; i < prefixes.size(); i++){
	// 	std::cout << prefixes[i] << std::endl;
	// }

	//format message
	//PREFIX/SUFFIX: (2|3)&n&INPUT&VALUE1&VALUE2&VALUE3...&VALUEn

	out += '2';
	out += '&';
	out.append(std::to_string(prefixes.size()));
	out += '&';
	out.append(inputWord);
	for(int i = 0; i < prefixes.size(); i++){
		out += '&';
		out.append(prefixes[i]);
	}
	out_c = new char[out.length()+1];
	strcpy(out_c,out.c_str());
	return(out_c);
}

char* suffix(char* input, suffixTree& suffix, char server){
	std::string inputWord;
	std::string out;
	char* out_c;
	int i = 2;
	while(input[i] != '\0'){
		inputWord += input[i];
		i++;
	}
	std::cout << "The server" << server << " received input <" << inputWord << "> and operation <suffix>" << std::endl;
	std::vector <std::string> suffixes = suffix.endsWith(inputWord);
	std::cout << "The Server" << server << " has found <" << suffixes.size() << "> matches" << std::endl; 


	//format message
	//PREFIX/SUFFIX: (2|3)&n&INPUT&VALUE1&VALUE2&VALUE3...&VALUEn

	out += '2';
	out += '&';
	out.append(std::to_string(suffixes.size()));
	out += '&';
	out.append(inputWord);
	for(int i = 0; i < suffixes.size(); i++){
		out += '&';
		out.append(suffixes[i]);
	}
	out_c = new char[out.length()+1];
	strcpy(out_c,out.c_str());
	return(out_c);
}


// int main(int argc, char const *argv[])
// {
// 	std::unordered_map <std::string, std::string> dictionary;
// 	prefixTree myTrie = prefixTree();
// 	suffixTree mySuf = suffixTree();
// 	startup("backendA.txt",dictionary,myTrie,mySuf);
// 	std::string searchWord;
// 	while(1){
// 		std::cout << "enter word to prefix:";
// 		std::cin >> searchWord;
// 		char* messageOut = prefix(&searchWord[0],myTrie,'A');
// 		std::cout << messageOut << std::endl;
// 	}
// 	return 0;
// }