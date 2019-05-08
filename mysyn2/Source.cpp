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

struct TOrNTStr { // terminal or non-terminal string
	vector<string> toks;
	int size() {
		return (int)toks.size();
	}
	string &operator[](int i) {
		if (i < 0 || i >= size())
			throw "TokenStr out of bounds!";
		return toks[i]; 
	}
	void push_back(const string t) {
		toks.push_back(t);
	}
};

struct Rules {
	map<string, vector<TOrNTStr>> rs; // non-terminal -> terminal or non-terminal string | ...
	enum class State { WAIT_FOR_NONT, WAIT_FOR_TS };
	Rules(string f) {
		ifstream fin(f);
		State state = State::WAIT_FOR_NONT;

		string nonTerminal;
		TOrNTStr ts;
		vector<TOrNTStr> expand;
		string s;
		while (fin >> s) {
			switch (state)
			{
			case State::WAIT_FOR_NONT: {
				nonTerminal = s;
				state = State::WAIT_FOR_TS;
			}
								break;
			case State::WAIT_FOR_TS: {
				if (s == "#") {
					expand.push_back(ts);
					rs[nonTerminal] = expand;
					nonTerminal = "";
					expand = vector<TOrNTStr>();
					state = State::WAIT_FOR_NONT;
				}
				else if (s == "::=") {
					ts = TOrNTStr();
				}
				else if (s == "|") {
					expand.push_back(ts);
					ts = TOrNTStr();
				}
				else {
					ts.push_back(s);
				}
			}
							  break;
			default:
				throw "Unrecognized State!";
			}
		}

		fin.close();
	}
};

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
	Rules r("rule.txt");
}
