#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <exception>
#include <cstring>
#include <set>
#include <stack>
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
	struct FollowSetHelper {
		// B includes A -> inclusiveGraph[A] contains B
		map<string, set<string>> inclusiveGraph;

		void init(Rules rs) {
			for (pair<string, set<TOrNTStr>> rule : rs.rs)
				inclusiveGraph[rule.first] = set<string>();
			for (pair<string, set<TOrNTStr>> rule : rs.rs) {
				string nonTerminal = rule.first;
				set<TOrNTStr> curExpansions = rule.second;
				for (TOrNTStr ts : curExpansions) {
					if (ts.size() == 0) {
						continue;
					}
					for (int i = 0; i < ts.size(); i++) {
						if (rs.rs.find(ts[i]) == rs.rs.end()) {
							// is terminal
							continue;
						}
						else {
							// is non-terminal
							if (i == ts.size() - 1) {
								// A->aB
								if (nonTerminal == ts[i]) {
									continue;
								}
								add(nonTerminal, ts[i]);
							}
							else {
								// A->aBb
								if (rs.rs.find(ts[i + 1]) == rs.rs.end()) {
									// b is terminal
									continue;
								}
								set<string> fSetb = rs.firstSets[ts[i + 1]];
								if (fSetb.find("") != fSetb.end()) {
									if (nonTerminal == ts[i]) {
										continue;
									}
									add(nonTerminal, ts[i]);
								}
							}
						}
					}
				}
			}
		}

		void add(string a, string b) {
			inclusiveGraph[a].insert(b);
		}

		bool isEqual(string a, string b) {
			if (a == b)
				return true;
			while (true) {
				vector<string> path;
				string p;
				stack<int> iStack;
				int i;
				p = a;
				i = 0;
				while (true) {
					bool bForceLeaf = false;
					if (path.size() > 0 && p == a) {
						// find a loop
						// for loop x -> y -> x, path is x -> y
						if (find(path.begin(), path.end(), b) != path.end()) {
							// this loop contains b
							return true;
						}
						else {
							// regard p as leaf node
							bForceLeaf = true;
						}
					}
					else if (path.size() > 0 && find(path.begin() + 1, path.end(), p) != path.end()) {
						// find a evil internal loop like x -> y -> z -> y, which will cause infinite loop
						// note path here is x -> y -> z
						// we thus regard p as leaf node
						bForceLeaf = true;
					}

					set<string> children = inclusiveGraph[p];
					if (children.size() == 0 || bForceLeaf) {
						// p is leaf node, go back
						while (true) {
							p = path[path.size() - 1];
							path.pop_back();
							i = iStack.top();
							iStack.pop();
							i++;
							set<string> children = inclusiveGraph[p];
							if (i != children.size())
								break;
							if (path.size() == 0)
								return false; // all path tried, no suitable loop found
						}
					} else {
						set<string>::iterator it = children.begin();
						advance(it, i);

						path.push_back(p);
						iStack.push(i);

						p = *it;
						i = 0;
					}
				}
			}
			// cannot find loop containing both a & b
			return false;
		}
	};

	map<string, set<TOrNTStr>> rs; // non-terminal -> terminal or non-terminal string | ...
	enum class State { WAIT_FOR_NONT, WAIT_FOR_TS };
	map<string, set<string>> firstSets, followSets;
	string startSym;
	FollowSetHelper followHelpler;
	Rules(string f) {
		ifstream fin(f);
		State state = State::WAIT_FOR_NONT;

		string nonTerminal;
		TOrNTStr ts;
		set<TOrNTStr> expand;
		string s;
		bool isFisrtLine;
		isFisrtLine = true;
		while (fin >> s) {
			switch (state)
			{
			case State::WAIT_FOR_NONT: {
				nonTerminal = s;
				state = State::WAIT_FOR_TS;				
				if (isFisrtLine) {
					isFisrtLine = false;
					startSym = nonTerminal;
				}
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

	void constructFollowSets() {
		followHelpler.init(*this);
		map<string, bool> bComplete;
		for (pair<string, set<TOrNTStr>> rule : rs) {
			bComplete[rule.first] = false;
			followSets[rule.first] = set<string>();
		}

		bool bNeedOnceMore;
		do {
			map<string, bool> bCompleteOptimistic;
			for (pair<string, set<TOrNTStr>> rule : rs) {
				bCompleteOptimistic[rule.first] = true;
			}
			bNeedOnceMore = false;
			for (pair<string, set<TOrNTStr>> rule : rs) {
				string nonTerminal = rule.first;
				set<TOrNTStr> curExpansions = rule.second;
				if (nonTerminal == startSym) {
					followSets[nonTerminal].insert(""); // means $
				}
				for (TOrNTStr ts : curExpansions) {
					if (ts.size() == 0) {
						continue;
					}
					for (int i = 0; i < ts.size(); i++) {
						if (rs.find(ts[i]) == rs.end()) {
							// is terminal
							continue;
						}
						else {
							// is non-terminal
							if (i == ts.size() - 1) {
								// A->aB
								if (followHelpler.isEqual(nonTerminal, ts[i])) {
									continue; 
								}
								followSets[ts[i]].insert(followSets[nonTerminal].begin(), followSets[nonTerminal].end());
								if (!bComplete[nonTerminal])
									bCompleteOptimistic[ts[i]] = false;
							}
							else {
								// A->aBb
								if (rs.find(ts[i + 1]) == rs.end()) {
									// b is terminal
									followSets[ts[i]].insert(ts[i + 1]);
									continue;
								}
								set<string> fSetb = firstSets[ts[i + 1]];
								if (fSetb.find("") != fSetb.end()) {
									if (followHelpler.isEqual(nonTerminal, ts[i])) {
										continue;
									}
									followSets[ts[i]].insert(followSets[nonTerminal].begin(), followSets[nonTerminal].end());
									if (!bComplete[nonTerminal])
										bCompleteOptimistic[ts[i]] = false;
								}
								fSetb.erase("");
								followSets[ts[i]].insert(fSetb.begin(), fSetb.end());
							}
						}
					}
				}
			}
			bComplete = bCompleteOptimistic;
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
	r.constructFollowSets();
}
