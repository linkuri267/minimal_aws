#include <string>
#include <cstring>
#include <iostream>
#include <vector>
#include <unordered_set>

std::string aggregateSearchMonitor(char* A, char* B, char* C, std::string& def, std::string& similar,bool& ASim, bool& BSim,bool& CSim){
    std::string out;
    std::string input;

    int i;
    if(A[2] == '&'){
 		i = 3;
    }
    else{
    	i = 2;
    }
    while((A[i] != '\0')&&(A[i] != '&')){
        input += A[i];
        i++;
    }
    std::string valueA;
    std::string valueB;
    std::string valueC;
    std::string wordA;
    std::string wordB;
    std::string wordC;
    std::string definitionA;
    std::string definitionB;
    std::string definitionC; 

    std::string value,word,definition;

    bool foundValue = false;
    bool foundWord = false;
    // std::cout << "Input: " << input << std::endl;
    // std::cout << "Value: " << value << std::endl;
    // std::cout << "Word: " << word << std::endl;
    // std::cout << "Definition: " << definition << std::endl;
    i = 2;
    if((A[0] == '1')&&(A[2] != '&')){
        foundValue = true;
        while(A[i] != '&'){
            i++;
        } 
        i++;
        while((A[i] != '&')&&(A[i] != '\0')){
            valueA += A[i];
            i++;
        }

        if(A[i] == '&'){
            i++;
            foundWord = true;
            ASim = true;
            while(A[i] != '&'){
                wordA += A[i];
                i++;
            }
            i++;
            while(A[i] != '\0'){
                definitionA += A[i];
                i++;
            }
        }
    }
    else if((A[0] == '1')&&(A[2] == '&')){
        ASim = true;
        foundWord = true;
        i = 3;
        while(A[i] != '&'){
            i++;
        } 
        i++;
        while(A[i] != '&'){
            wordA += A[i];
            i++;
        }
        i++;
        while(A[i] != '\0'){
            definitionA += A[i];
            i++;
        }
    }

    i = 2;
    if((B[0] == '1')&&(B[2] != '&')){
        foundValue = true;
        while(B[i] != '&'){
            i++;
        } 
        i++;
        while((B[i] != '&')&&(B[i] != '\0')){
            valueB += B[i];
            i++;
        }
        if((B[i] == '&')){
            BSim = true;
            i++;
            foundWord = true;
            while(B[i] != '&'){
                wordB += B[i];
                i++;
            }
            i++;
            while(B[i] != '\0'){
                definitionB += B[i];
                i++;
            }
        }
    }
    else if((B[0] == '1')&&(B[2] == '&')){
        BSim = true;
        foundWord = true;
        i = 3;
        while(B[i] != '&'){
            i++;
        } 
        i++;
        while(B[i] != '&'){
            wordB += B[i];
            i++;
        }
        i++;
        while(B[i] != '\0'){
            definitionB += B[i];
            i++;
        }
    }

    i = 2;
    if((C[0] == '1')&&(C[2] != '&')){
        foundValue = true;
        while(C[i] != '&'){
            i++;
        } 
        i++;
        while((C[i] != '&')&&(C[i] != '\0')){
            valueC += C[i];
            i++;
        }
        if((C[i] == '&')){
            CSim = true;
            i++;
            foundWord = true;
            while(C[i] != '&'){
                wordC += C[i];
                i++;
            }
            i++;
            while(C[i] != '\0'){
                definitionC += C[i];
                i++;
            }
        }
    }
    else if((C[0] == '1')&&(C[2] == '&')){
        CSim = true;
        foundWord = true;
        i = 3;
        while(C[i] != '&'){
            i++;
        } 
        i++;
        while(C[i] != '&'){
            wordC += C[i];
            i++;
        }
        i++;
        while(C[i] != '\0'){
            definitionC += C[i];
            i++;
        }
    }
    if(valueA != ""){
        value = valueA;
    }
    else if(wordB != ""){
        value = valueB;
    }
    else if(wordC != ""){
        value = value;
    }
    if(wordA != ""){
        word = wordA;
        definition = definitionA;
    }
    else if(wordB != ""){
        word = wordB;
        definition = definitionB;
    }
    else if(wordC != ""){
        word = wordC;
        definition = definitionC;
    }

    // std::cout << "Input: " << input << std::endl;
    // std::cout << "Value: " << value << std::endl;
    // std::cout << "Word: " << word << std::endl;
    // std::cout << "Definition: " << definition << std::endl;

    if(value == ""){
        if(word == ""){
            out = "0&";
            out.append(input);
        }
        else{
            out = "1&&";
            out.append(input);
            out += '&';
            out.append(word);
            out += '&';
            out.append(definition);
        }
    }
    else{
        if(word == ""){
            out = "1&";
            out.append(input);
            out += '&';
            out.append(value);
        }
        else{
            out = "1&";
            out.append(input);
            out += '&';
            out.append(value);
            out += '&';
            out.append(word);
            out += '&';
            out.append(definition);
        }
    }
    def = value;
    similar = word;
    return(out);
}
       //Output format (from backend):
        //DIDN'T FIND ANYTHING: 0&INPUT
        //SEARCH AND FOUND SIMILAR: 1&INPUT&VALUE&WORD&DEFINITION
        //SEARCH AND DIDN'T FIND SIMILAR: 1&INPUT&VALUE
        //DIDNT FIND BUT FOUND SIMILAR: 1&&INPUT&WORD&DEFINITION
        //PREFIX/SUFFIX: (2|3)&n&INPUT&VALUE1&VALUE2&VALUE3...&VALUEn

        //Message format (to client):
        //If not found: '&\0'
        //If search: 'VALUE\0'
        //If prefix or suffix: 'n&VALUE1&VALUE2&VALUE3...&VALUEn\0'

std::string aggregateSearchClient(char* A, char* B, char* C){
	std::string out;
	if(((A[0] == '0')||((A[1] == '&')&&(A[2] == '&')))&&((B[0] == '0')||((B[1] == '&')&&(B[2] == '&')))&&((C[0] == '0')||((C[1] == '&')&&(C[2] == '&')))){
		out = "&";
	}
	else{
		int i = 2;
		if((A[0] != '0')&&(A[2] != '&')){
			while(A[i] != '&'){
				i++;
			}
			i++;
			while((A[i] != '&')&&(A[i] != '\0')){
				out += A[i];
				i++;
			}
		}
		else if((B[0] != '0')&&(B[2] != '&')){
			while(B[i] != '&'){
				i++;
			}
			i++;
			while((B[i] != '&')&&(B[i] != '\0')){
				out += B[i];
				i++;
			}
		}
		else if((C[0] != '0')&&(C[2] != '&')){
			while(C[i] != '&'){
				i++;
			}
			i++;
			while((C[i] != '&')&&(C[i] != '\0')){
				out += C[i];
				i++;
			}
		}
	}
	return(out);
}

//0&INPUT
//PREFIX/SUFFIX: (2|3)&n&INPUT&VALUE1&VALUE2&VALUE3...&VALUEn

        //Message format (to client):
        //If not found: '&\0'
        //If search: 'VALUE\0'
        //If prefix or suffix: 'n&VALUE1&VALUE2&VALUE3...&VALUEn\0'
std::string aggregatePrefixSuffixClient(char* A, char* B, char* C){
    std::string temp;
    std::string input;

    std::unordered_set <std::string> values;

    if(A[0] != '0'){
        int i = 2;
        while(A[i] != '&'){
            i++;
        }
        i++;
        
        while(A[i] != '&'){
            if(A[i] == '\0'){
                i--;
                break;
            }
            input += A[i];
            i++;
        }
        i++;
        temp = "";
        while(A[i] != '\0'){
            if(A[i] == '&'){
                if(values.find(temp) == values.end()){
                    values.insert(temp);
                }
                temp = "";
            }
            else{
                temp += A[i];
            }
            i++;
        }
        if((values.find(temp) == values.end())&&(temp != "")){
            values.insert(temp);
        }
    }
    else{
        int i = 2;
        while(A[i] != '\0'){
            temp += A[i];
            i++;
        }
        input = temp;
    }
    temp = "";
    if(B[0] != '0'){
        int i = 2;
        while(B[i] != '&'){
            i++;
        }
        i++;
        while(B[i] != '&'){
            if(B[i] == '\0'){
                i--;
                break;
            }
            i++;
        }
        i++;
        temp = "";
        while(B[i] != '\0'){
            if(B[i] == '&'){
                if(values.find(temp) == values.end()){
                    values.insert(temp);
                }
                temp = "";
            }
            else{
                temp += B[i];
            }
            i++;
        }
        if((values.find(temp) == values.end())&&(temp != "")){
            values.insert(temp);
        }
    }
    temp = "";
    if(C[0] != '0'){
        int i = 2;
        while(C[i] != '&'){
            i++;
        }
        i++;
        while(C[i] != '&'){
            if(C[i] == '\0'){
                i--;
                break;
            }
            i++;
        }
        i++;
        temp = "";
        while(C[i] != '\0'){
            if(C[i] == '&'){
                if(values.find(temp) == values.end()){
                    values.insert(temp);
                }
                temp = "";
            }
            else{
                temp += C[i];
            }
            i++;
        }
        if((values.find(temp) == values.end())&&(temp != "")){
            values.insert(temp);
        }
    }


    std::string out;
    if(values.size() == 0){
        out = "&";
    }
    else{
        out.append(std::to_string(values.size()));
    }      

    for(auto it = values.begin(); it != values.end(); ++it){
        out += '&';
        out += *it;
    }

    return(out);



}

std::string aggregatePrefixSuffixMonitor(char* A, char* B, char* C,int& cA,int& cB, int& cC){
    std::string temp;
    std::string input;
    char type;

    std::unordered_set <std::string> values;

    if(A[0] != '0'){
        type = A[0];
        int i = 2;
        while(A[i] != '&'){
            i++;
        }
        i++;
        
        while(A[i] != '&'){
            if(A[i] == '\0'){
                i--;
                break;
            }
            input += A[i];
            i++;
        }
        i++;
        temp = "";
        while(A[i] != '\0'){
            if(A[i] == '&'){
                cA ++;
                if(values.find(temp) == values.end()){
                    values.insert(temp);
                }
                temp = "";
            }
            else{
                temp += A[i];
            }
            i++;
        }
        if(temp != ""){
            cA ++;
        }
        if((values.find(temp) == values.end())&&(temp != "")){
            values.insert(temp);
        }
    }
    else{
        int i = 2;
        while(A[i] != '\0'){
            temp += A[i];
            i++;
        }
        input = temp;
    }
    temp = "";
    if(B[0] != '0'){
        int i = 2;
        while(B[i] != '&'){
            i++;
        }
        i++;
        while(B[i] != '&'){
            if(B[i] == '\0'){
                i--;
                break;
            }
            i++;
        }
        i++;
        temp = "";
        while(B[i] != '\0'){
            if(B[i] == '&'){
                cB ++;
                if(values.find(temp) == values.end()){
                    values.insert(temp);
                }
                temp = "";
            }
            else{
                temp += B[i];
            }
            i++;
        }
        if(temp != ""){
            cB ++;
        }
        if((values.find(temp) == values.end())&&(temp != "")){
            values.insert(temp);
        }
    }
    temp = "";
    if(C[0] != '0'){
        int i = 2;
        while(C[i] != '&'){
            i++;
        }
        i++;
        while(C[i] != '&'){
            if(C[i] == '\0'){
                i--;
                break;
            }
            i++;
        }
        i++;
        temp = "";
        while(C[i] != '\0'){
            if(C[i] == '&'){
                cC ++;
                if(values.find(temp) == values.end()){
                    values.insert(temp);
                }
                temp = "";
            }
            else{
                temp += C[i];
            }
            i++;
        }
        if(temp != ""){
            cC ++;
        }
        if((values.find(temp) == values.end())&&(temp != "")){
            values.insert(temp);
        }
    }
    std::string out;
    if(values.size() == 0){
        out += '0';
        out += '&';
        out.append(input);
        return(out);
    }

    out += type;
    out += '&';
    out.append(std::to_string(values.size()));
    out += '&';
    out.append(input);

    for(auto it = values.begin(); it != values.end(); ++it){
        out += '&';
        out += *it;
    }


    return(out);
}

// int main(int argc, char const *argv[])
// {
// 	char* A,*B,*C;
// 	A = new char[sizeof("2&4&Pall&Pall&Pall&Palla&Palladian")];
// 	B = new char[sizeof("2&2&Pall&Palladium&Pallium")];
// 	C = new char[sizeof("2&3&Pall&Pallet&Palliard&Pallid")];

// 	strcpy(A,"2&4&Pall&Pall&Pall&Palla&Palladian");
// 	strcpy(B,"2&2&Pall&Palladium&Pallium");
// 	strcpy(C,"2&3&Pall&Pallet&Palliard&Pallid");
// 	std::cout << aggregatePrefixSuffixMonitor(A,B,C) << std::endl;

// 	return 0;
// }