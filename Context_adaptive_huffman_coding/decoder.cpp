#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <bitset>

using namespace std;

int binary_int(string b) {// Receive a string written in binary and convert it to a decimal value.
	int bi = stoi(b);
	int result = 0, mul = 1;
	while (bi > 0) {//repeat if bi is greater than 0
		if ((bi % 2) == 1) { result += mul; }//If the remainder is 1, add it.
		mul *= 2;
		bi /= 10;
	}
	return result;
}

int main() {
	map<string, char>huffman;//Decoded table
	map<string, char>adapt_huffman[128];//Decoded adapt_table

	//Decoding a generic Huffman table
	ifstream readfile("huffman_table.hbs", ifstream::binary);
	unsigned char* table = NULL;
	int len = 0;
	if (readfile) {//Reading binary files
		readfile.seekg(0, readfile.end);
		len = (int)readfile.tellg();
		readfile.seekg(0, readfile.beg);

		table = (unsigned char*)malloc(len);

		readfile.read((char*)table, len);
		readfile.close();
	}

	//Receives in 1 byte unit, reads it in 8 bits, and stores it in str string
	bitset<8> b;
	string str = "\0";
	for (int i = 0; i < len; i++) {
		b = table[i];
		str += b.template to_string<char, char_traits<char>, allocator<char> >();
	}

	//Create table using str
	char ascii;
	int index = 0, bitnum = 0;
	string cw;
	while (index + 8 < str.length()) {//Repeat if next letter is found
		ascii = (char)binary_int(str.substr(index, 8));	//Receiving text
		index += 8;
		bitnum = binary_int(str.substr(index, 8));	//Get how many bits a codeword is
		index += 8;
		cw = str.substr(index, bitnum);		//save codeword
		index += bitnum;
		huffman.insert(pair<string, char>(cw, ascii));	//insert into huffman table
	}

	//eof find and save
	string eof;
	for (map<string, char>::iterator it = huffman.begin(); it != huffman.end(); it++) {
		if ((int)it->second == 26) { eof = it->first; break; }//Store eof codeword in eof and repeat
	}

	//adaptive Huffman table decoding
	ifstream readfile1("context_adaptive_huffman_table.hbs", ifstream::binary);
	unsigned char* adapt_table = NULL;
	int len1 = 0;
	if (readfile1) {//Reading binary files
		readfile1.seekg(0, readfile1.end);
		len1 = (int)readfile1.tellg();
		readfile1.seekg(0, readfile1.beg);

		adapt_table = (unsigned char*)malloc(len1);

		readfile1.read((char*)adapt_table, len1);
		readfile1.close();
	}

	//Receives in 1 byte unit, reads it in 8 bits, and stores it in str string
	string adapt_str = "\0";
	for (int i = 0; i < len1; i++) {
		b = adapt_table[i];
		adapt_str += b.template to_string<char, char_traits<char>, allocator<char> >();
	}


	//Create table using str
	index = 0;
	bitnum = 0;
	string cw1;
	int tablenum = 0, prevnum = 0;
	tablenum = binary_int(adapt_str.substr(index, 8));//Get number of tables
	index += 8;
	for (int i = 0; i < tablenum; i++) {//Repeat the number of tables
		prevnum = binary_int(adapt_str.substr(index, 8));//Get previous character
		index += 8;
		while ((adapt_str.substr(index, eof.length())) != eof) {
			ascii = (char)binary_int(adapt_str.substr(index, 8));	//Receiving text
			index += 8;
			bitnum = binary_int(adapt_str.substr(index, 8));	//Get how many bits a codeword is
			index += 8;
			cw1 = adapt_str.substr(index, bitnum);		//save codeword
			index += bitnum;
			adapt_huffman[prevnum].insert(pair<string, char>(cw1, ascii));	//insert into huffman table
		}
		index += eof.length();
	}

	//Receive codeword and create original
	string fi[4] = { "training_input_code.hbs","test_input1_code.hbs","test_input2_code.hbs","test_input3_code.hbs" };
	string fo[4] = { "training_output.txt","test_output1.txt","test_output2.txt","test_output3.txt" };

	for (int q = 0; q < 4; q++) {
		ifstream codefile(fi[q], ifstream::binary);
		unsigned char* code = NULL;
		int codelen = 0;
		if (codefile) {//Reading code binary files
			codefile.seekg(0, codefile.end);
			codelen = (int)codefile.tellg();
			codefile.seekg(0, codefile.beg);

			code = (unsigned char*)malloc(codelen);

			codefile.read((char*)code, codelen);
			codefile.close();
		}

		str = "\0";
		for (int i = 0; i < codelen; i++) {//Read 1 byte and convert to bit type string
			b = code[i];
			str += b.template to_string<char, char_traits<char>, allocator<char> >();
		}

		index = 0;
		int i = 1;
		string sub, result = "\0";
		map<string, char>::iterator it;

		//Decoding using adaptive
		int prev = 0;
		while (1) {//loop
			sub = str.substr(index, i);
			if (prev != 0) {//If not the first character
				if (adapt_huffman[prev].size() != 0) {//When to use the old table
					if ((it = adapt_huffman[prev].find(sub)) != adapt_huffman[prev].end()) {//in the adaptive huffman table
						if (it->second == 26) { break; }
						result += it->second;
						index += i;
						i = 1;
						prev = it->second;//Save previous character
					}
					else { i++; }
				}
				else if ((it = huffman.find(sub)) != huffman.end()) {//If you have a regular Huffman at the table
					if (it->second == 26) { break; }//End iteration if eof
					result += it->second;
					index += i;
					i = 1;
					prev = it->second;//Save previous character
				}
				else { i++; }//Not in any table
			}
			else {//If it is the first character
				it = huffman.find(sub);
				if (it == huffman.end()) { i++; }//If it is a codeword that is not in the table
				else {
					if (it->second == 26) { break; }//End iteration if eof
					else {//If it is a letter
						result += it->second;
						index += i;
						i = 1;
						prev = it->second;//Save previous character
					}
				}
			}
		}

		ofstream writefile;
		writefile.open(fo[q]);
		if (writefile.is_open()) {//Output as txt file
			writefile.write(result.c_str(), result.length());//write
		}
		writefile.close();
	}
	return 0;
}