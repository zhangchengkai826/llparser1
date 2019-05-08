#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <exception>
#include <cstring>
using namespace std;

struct Token {
	int k;
	int v;
	bool operator<(const Token &t) const {
		if (k < t.k)
			return true;
		else if (k > t.k)
			return false;
		else {
			if (v < t.v)
				return true;
			else if (v > t.v)
				return false;
		}
		return false; // two Tokens are identical
	}
};

map<Token, string> tltb; // token -> terminal

struct TokenStr {
	vector<Token> toks;
	int size() {
		return (int)toks.size();
	}
	Token &operator[](int i) {
		if (i < 0 || i >= size())
			throw "TokenStr out of bounds!";
		return toks[i]; 
	}
};

map<string, vector<TokenStr>> rules; // non-terminal -> (non-)terminal strings | ...

void readTltb(string f) {
	ifstream fin(f);

	while (true) {
		string terminal;
		Token token;
		string vexpr; // Token.v expression

		fin >> terminal >> token.k >> vexpr;
		if (!fin)break;
		if (vexpr.find('x') != string::npos) {
			token.v = -1;
			tltb[token] = terminal;
		}
		else if (vexpr.find('-') != string::npos) {
			int pos = (int)vexpr.find('-');
			int s = atoi(vexpr.substr(0, pos).c_str());
			int e = atoi(vexpr.substr(pos+1, string::npos).c_str());
			for (int i = s; i <= e; i++) {
				token.v = i;
				tltb[token] = terminal;
			}
		}
		else {
			token.v = atoi(vexpr.c_str());
			tltb[token] = terminal;
		}
	}

	fin.close();
}

int main() {
	readTltb("tltb.txt");
	
}