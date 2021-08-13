#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>		
#include <map>
#include <stdio.h>
#include <bitset>
using namespace std;

typedef struct node {//Node with information about the character
	char symbol; //character
	double probability;
	string codeword;

	node* left;
	node* right;
	node* par;

	node(char s, double p) {//Default constructor
		symbol = s;
		probability = p;
		codeword = "\0";
		left = NULL;
		right = NULL;
		par = NULL;
	}

	node(char s, double p, node* l, node* r) {
		symbol = s;
		probability = p;
		codeword = "\0";
		if (l->probability == r->probability && r->symbol == 1) { this->left = r; this->right = l; }
		else { this->left = l; this->right = r; }
		par = NULL;
		l->par = this;
		r->par = this;
	}
	~node() { delete left, right, par; };
}node;

typedef struct tree {
	node* root;

	tree() { root = NULL; }
	~tree() { delete root; }
}tree;

void preorder(node* c, map<char, pair<string, double>>* count) {//Codeword generation through front-end traversal
	if (c == NULL) return;
	if (c->left != NULL && c->left->codeword == "\0") { c->left->codeword = c->codeword + "0"; }//Put 0 on the left
	if (c->right != NULL && c->right->codeword == "\0") { c->right->codeword = c->codeword + "1"; }//Put 1 on the right
	if (c->left == NULL && c->right == NULL) { count->find(c->symbol)->second.first = c->codeword; }//If a node has no children, save the codeword in the map.
	preorder(c->left, count);
	preorder(c->right, count);
}

unsigned char fit(const char* arr) {
	unsigned char pa = 0;
	int j = 7;
	for (int i = 0; i <= 7; i++) {//8bit fit
		if (arr[i] == '1') {//If the value is 1 at the position of the array passed as an argument
			pa |= 1 << j; //Align 1 position
		}
		j--;//size to shift
	}
	return pa;
}

int main() {
	map<char, pair<string, double>> adapt[128];	//adaptive huffman table
	multimap<double, node*> adaptprob[128];	//Multimap sorting probabilities by previous character
	int allpn[128] = { 0, };	//how many value s were in the previous chracter
	int prev = 0;	//ASCII code value of previous character

	map<char, pair<string, double>> count;
	map<char, pair<string, double>>::iterator it;	//iterator to receive the position of count
	multimap<double, node*> prob;	//Multimap sorted in ascending order of probability
	multimap<double, node*>::iterator iter;	//iterator to receive the position of the multimap

	char sym;		//Characters read
	double st_num = 0;	//Total number of characters

	tree* huffman = new tree;
	tree* adapthuffman = new tree;
	ofstream output;

	FILE* file = NULL;
	file = fopen("training_input.txt", "r"); //Open in reading mode

	//Counting the number of characters
	if (file == NULL) { return 0; }//error
	else {
		while ((sym = fgetc(file)) != EOF) {//Eof doesn't store the last data because ASCII code 26 needs to be known.
			//Counting to create a regular Huffman tree
			it = count.find(sym);//Finding if a character already exists
			if (it == count.end()) { count.insert(pair<char, pair<string, double>>(sym, make_pair<string, double>("\0", 1))); }	//It is 1 because it is the first letter.
			else { it->second.second += 1; }//count
			st_num++;	//Add 1 character

			//Counting the current number of characters for the previous character to generate an adaptive Huffman tree
			it = adapt[prev].find(sym);	//Check if it has already been inserted at the previous character position
			if (it == adapt[prev].end()) { adapt[prev].insert(pair<char, pair<string, double>>(sym, make_pair<string, double>("\0", 1))); }//If a character has not been entered, it is added as number 1
			else { it->second.second += 1; }//count
			allpn[prev] += 1;	//How many times the value for the previous character appears
			prev = sym;	//Save the current character to the previous character (the next character becomes the previous character)
		}
		fclose(file);
	}

	//put eof
	adapt[prev].insert(pair<char, pair<string, double>>(26, make_pair<string, double>("\0", 1)));
	allpn[prev] ++;
	count.insert(pair<char, pair<string, double>>(26, make_pair<string, double>("\0", 1)));	//EOF added at the end
	st_num++;

	//Add all regular Huffman tableless characters
	for (int i = 0; i < 128; i++) {
		map<char, pair<string, double>>::iterator itt = count.find((char)i);
		if (itt == count.end()) {//if the character is not in count
			count.insert(pair<char, pair<string, double>>((char)i, make_pair<string, double>("\0", 1)));
		}
		else { itt->second.second++; }//Characters with frequency increase by 1
		st_num++;
	}

	//adaptive Huffman table Add current character by previous character All characters
	for (int i = 0; i < 128; i++) {
		if (adapt[i].size() != 0) {//If you use the previous character table
			for (int k = 0; k < 128; k++) {
				map<char, pair<string, double>>::iterator itt = adapt[i].find((char)k);
				if (itt == adapt[i].end()) {//if the character is not in adapt[i]
					adapt[i].insert(pair<char, pair<string, double>>((char)k, make_pair<string, double>("\0", 1)));
				}
				else { itt->second.second++; }//Characters with frequency increase by 1
				allpn[i]++;
			}
		}
	}

	map<char, pair<string, double>>::iterator it1;
	int adaptcount = 0;	//Count how many adapt tables are used.
	double avr = st_num / count.size();
	for (it = count.begin(); it != count.end(); it++) {
		if (avr / 5 <= it->second.second) {//Use previous characters with frequency of average/5 or higher
			int ine = (int)it->first;
			adaptcount += 1;//count up
			for (it1 = adapt[ine].begin(); it1 != adapt[ine].end(); it1++) {
				double p1 = it1->second.second / allpn[ine];
				it1->second.second = p1;
				node* pr1 = new node(it1->first, p1);	//Create a new node
				adaptprob[ine].insert(pair<double, node*>(p1, pr1));
			}
		}
		double p = it->second.second / st_num;	//Save probabilities for each character
		it->second.second = p;
		node* pr = new node(it->first, p);	//Create a new node
		prob.insert(pair<double, node*>(p, pr));
	}

	//Building a Huffman Tree
	node* parent, * l, * r;	//Put the value to the left and right of parent
	while (prob.size() != 1) {	//Until there is 1 left in the prob
		l = prob.begin()->second;
		prob.erase(prob.begin());	//Delete minimum value
		r = prob.begin()->second;
		prob.erase(prob.begin());	//Delete minimum value
		parent = new node(1, l->probability + r->probability, l, r);	//Initialization of new parent node (character is ASCII code 1 value)
		prob.insert(pair<double, node*>(parent->probability, parent));	//insert new node into prob
	}

	huffman->root = prob.begin()->second;//Initialize root
	preorder(huffman->root, &count);	//Number mapping

	//Creating an adaptive huffman table
	for (int q = 0; q < 128; q++) {
		if (adaptprob[q].size() != 0) {
			if (adaptprob[q].size() == 1) { //When the current value for the previous value is 1
				adapt[q].find(adaptprob[q].begin()->second->symbol)->second.first = "0";
			}
			else {
				while (adaptprob[q].size() > 1) {	//Until there is 1 left in the prob
					l = adaptprob[q].begin()->second;
					adaptprob[q].erase(adaptprob[q].begin());	//Delete minimum value
					r = adaptprob[q].begin()->second;
					adaptprob[q].erase(adaptprob[q].begin());	//Delete minimum value
					parent = new node(1, l->probability + r->probability, l, r);	//Initialization of new parent node (character is ASCII code 1 value)
					adaptprob[q].insert(pair<double, node*>(parent->probability, parent));	//insert new node into prob
				}
				adapthuffman->root = adaptprob[q].begin()->second;//Initialize root
				preorder(adapthuffman->root, &adapt[q]);	//Number mapping
			}
		}
	}

	//Normal table encoding
	bitset<8> b;//8bit bitstream
	string table = "\0";
	for (it = count.begin(); it != count.end(); it++) {
		b = it->first;	//Change char to 8bit bitstream
		table += b.template to_string<char, char_traits<char>, allocator<char> >();
		b = it->second.first.length();
		table += b.template to_string<char, char_traits<char>, allocator<char> >();
		table += it->second.first;
	}
	output.open("huffman_table.hbs", ios::binary);

	const char* ta = table.c_str();	//convert string to const char*
	for (int i = 0; i * 8 < table.length(); i++) {//Divide the size by 8 and write it in 1 byte size.
		char re = fit(&table[i * 8]);
		output.write(&re, sizeof(re));
	}
	output.close();

	//adaptive Huffman table encoding
	string adapt_table = "\0";
	b = adaptcount;//Number of tables used
	adapt_table += b.template to_string<char, char_traits<char>, allocator<char> >();
	for (int i = 0; i < 128; i++) {//Check all previous characters
		if (adapt[i].size() != 0 && adapt[i].begin()->second.first != "\0") {//For a table with previous characters
			b = i;
			adapt_table += b.template to_string<char, char_traits<char>, allocator<char> >();
			for (it = adapt[i].begin(); it != adapt[i].end(); it++) {//If it is a previous table to be used, it is repeated as many times as the number of character types
				b = it->first;	//Change char to 8bit bitstream
				adapt_table += b.template to_string<char, char_traits<char>, allocator<char> >();
				b = it->second.first.length();
				adapt_table += b.template to_string<char, char_traits<char>, allocator<char> >();
				adapt_table += it->second.first;
			}
			it = count.find(26);
			adapt_table += it->second.first;//Add EOF
		}
	}
	output.open("context_adaptive_huffman_table.hbs", ios::binary);

	int padding = (8 - (adapt_table.length() % 8)) % 8;//Calculate stuffing bit according to 8bit
	for (int i = 0; i < padding; i++) { adapt_table += "0"; }   //Fill with zeros

	ta = adapt_table.c_str();	//convert string to const char*
	for (int i = 0; i * 8 < adapt_table.length(); i++) {//Divide the size by 8 and write it in 1 byte size.
		char re = fit(&adapt_table[i * 8]);
		output.write(&re, sizeof(re));
	}
	output.close();

	//Encoding using an adaptive table
	string fi[4] = { "training_input.txt","test_input1.txt","test_input2.txt","test_input3.txt" };
	string fo[4] = { "training_input_code.hbs","test_input1_code.hbs","test_input2_code.hbs","test_input3_code.hbs" };
	for (int q = 0; q < 4; q++) {
		int first = 0;
		string result = "\0";	//Result string
		file = fopen(fi[q].c_str(), "r");
		prev = 0; //The first preceding character is NULL.
		if (file == NULL) { return 0; }//error
		else {
			while ((sym = fgetc(file)) != EOF) {
				if (first != 0) {//If it is not the first letter
					if (adapt[prev].size() != 0 && adapt[prev].begin()->second.first != "\0") {//If there is a previous character in the table
						if ((it = adapt[prev].find(sym)) != adapt[prev].end()) {//If there is the current character you are looking for
							result += it->second.first;//Add to result
							prev = sym;
							continue;//Go to loop
						}
					}
					it = count.find(sym);//Use of regular tables
					result += it->second.first;//Add to result
					prev = sym;
				}
				else {//First character
					first = 1;
					result += count.find(sym)->second.first;
					prev = sym;
				}
			}
			if (adapt[prev].size() != 0 && adapt[prev].begin()->second.first != "\0") {//If there is a previous character in the table
				if ((it = adapt[prev].find(26)) != adapt[prev].end()) {//If there is the current character you are looking for
					result += it->second.first;//Add to result
				}
			}
			else {
				it = count.find(26);	//check the last eof
				result += it->second.first;	//Put the last eof
			}
			fclose(file);
		}

		padding = (8 - (result.length() % 8)) % 8;//Calculate stuffing bit according to 8bit
		for (int i = 0; i < padding; i++) { result += "0"; }   //Fill with zeros

		output.open(fo[q], ios::binary);

		const char* result_str = result.c_str();	// convert string to const char*
		for (int i = 0; i * 8 < result.length(); i++) {//Divide the size by 8 and write it in 1 byte size.
			char re = fit(&result_str[i * 8]);
			output.write(&re, sizeof(re));
		}

		output.close();
	}

	return 0;
}