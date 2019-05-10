#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <exception>
#include <cstring>
#include <set>
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
	int size() const {
		return (int)toks.size();
	}
	bool isEmpty() const {
		if (size() < 0)
			throw "TokenStr out of bounds!";
		else if (size() == 0)
			return true;
		else
			return false;
	}
	string &operator[](int i) {
		if (i < 0 || i >= size())
			throw "TokenStr out of bounds!";
		return toks[i];
	}
	const string &operator[](int i) const {
		if (i < 0 || i >= size())
			throw "TokenStr out of bounds!";
		return toks[i];
	}
	void push_back(const string t) {
		toks.push_back(t);
	}
	void eraseAt(int i) {
		toks.erase(toks.begin() + i);
	}
	void insertTOrNTStrAt(int i, const TOrNTStr t) {
		toks.insert(toks.begin() + i, t.toks.begin(), t.toks.end());
	}
	bool operator<(const TOrNTStr ts) const {
		if (size() < ts.size())
			return true;
		else if (size() > ts.size())
			return false;
		else {
			for (int i = 0; i < size(); i++) {
				if (toks[i] < ts.toks[i])
					return true;
				else if (toks[i] > ts.toks[i])
					return false;
			}
			return false; // equal
		}
	}
	bool operator>(const TOrNTStr ts) const {
		if (size() > ts.size())
			return true;
		else if (size() < ts.size())
			return false;
		else {
			for (int i = 0; i < size(); i++) {
				if (toks[i] > ts.toks[i])
					return true;
				else if (toks[i] < ts.toks[i])
					return false;
			}
			return false; // equal
		}
	}
};

struct Rules {
	map<string, set<TOrNTStr>> rs; // non-terminal -> terminal or non-terminal string | ...
	enum class State { WAIT_FOR_NONT, WAIT_FOR_TS };
	map<string, set<string>> firstSets, followSets;
	Rules(string f) {
		ifstream fin(f);
		State state = State::WAIT_FOR_NONT;

		string nonTerminal;
		TOrNTStr ts;
		set<TOrNTStr> expand;
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
					expand.insert(ts);
					rs[nonTerminal] = expand;
					nonTerminal = "";
					expand = set<TOrNTStr>();
					state = State::WAIT_FOR_NONT;
				}
				else if (s == "::=") {
					ts = TOrNTStr();
				}
				else if (s == "|") {
					expand.insert(ts);
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
	void removeLeftRecoursion() {
		map<string, set<TOrNTStr>> checked;
		for (pair<string, set<TOrNTStr>> rule : rs) {
			string nonTerminal = rule.first;
			string altNonTerminal; // added for removing left recursion
			set<TOrNTStr> curExpansions = rule.second;
			for (pair<string, set<TOrNTStr>> cRule : checked) {
				set<TOrNTStr> nCurExpansions;
				for (TOrNTStr ts : curExpansions) {
					if (ts.isEmpty() || ts[0] != cRule.first) 
						nCurExpansions.insert(ts);
					else {
						// should expand
						ts.eraseAt(0);
						for (TOrNTStr possibleCExpand : cRule.second) {
							TOrNTStr nts = ts;
							nts.insertTOrNTStrAt(0, possibleCExpand);
							nCurExpansions.insert(nts);
						}
					}
				}
				curExpansions = nCurExpansions;
			}
			// curExpansions now fully expands, start removing immediate left recursion...
			// check for immediate left recursion...
			bool bHasImmLeftRecursion = false;
			for (const TOrNTStr &ts : curExpansions) {
				if (ts.isEmpty()) continue;
				if (ts[0] == nonTerminal) {
					bHasImmLeftRecursion = true;
					break;
				}
			}
			if (bHasImmLeftRecursion) {
				// remove immediate left recursion...
				string origNonterminal = nonTerminal;
				string altNonTerminal = nonTerminal + "Alt";
				set<TOrNTStr> orig;
				set<TOrNTStr> alt;

				for (TOrNTStr ts : curExpansions) {
					if (ts.isEmpty()) continue;
					if (ts[0] == nonTerminal) {
						TOrNTStr nts = ts;
						nts.eraseAt(0);
						nts.push_back(altNonTerminal);
						alt.insert(nts);
					}
					else {
						TOrNTStr nts = ts;
						nts.push_back(altNonTerminal);
						orig.insert(nts);
					}
				}
				alt.insert(TOrNTStr()); // append an empty expansion

				checked[origNonterminal] = orig;
				checked[altNonTerminal] = alt;
			}
			else {
				checked[nonTerminal] = curExpansions;
			}
		}
		rs = checked;
	}
	
	void constructFirstSets() {
		map<string, bool> bComplete;
		for (pair<string, set<TOrNTStr>> rule : rs) 
			bComplete[rule.first] = false;
		bool bNeedOnceMore;
		do {
			bNeedOnceMore = false;
			for (pair<string, set<TOrNTStr>> rule : rs) {
				string nonTerminal = rule.first;
				set<TOrNTStr> curExpansions = rule.second;
				set<string> fSet;
				bool bIsThisNonTerminalComplete = true;
				for (TOrNTStr ts : curExpansions) {
					if (ts.size() == 0) {
						fSet.insert("");
						continue;
					}
					for (string t : ts.toks) {
						if (rs.find(t) == rs.end()) {
							// is terminal
							fSet.insert(t);
							break;
						}
						else {
							// is non-terminal
							if (t != nonTerminal) {
								if (firstSets.find(t) != firstSets.end())
									fSet.insert(firstSets[t].begin(), firstSets[t].end());
								if (!bComplete[t])
									bIsThisNonTerminalComplete = false; // In best situation, this code never gets executed!
							}

							set<TOrNTStr> expands = rs[t];
							bool bHasEmptyExpand = false;
							for (TOrNTStr e : expands)
								if (e.size() == 0)
									bHasEmptyExpand = true;
							if (!bHasEmptyExpand)
								break;
						}
					}
				}
				firstSets[nonTerminal] = fSet;
				bComplete[nonTerminal] = bIsThisNonTerminalComplete;
			}
			for (pair<string, set<TOrNTStr>> rule : rs)
				if (!bComplete[rule.first])
					bNeedOnceMore = true;
		} while (bNeedOnceMore);
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
	r.removeLeftRecoursion();
	r.constructFirstSets();
}
