#ifndef BRANCH_PREDICTOR_H
#define BRANCH_PREDICTOR_H

#include <iostream>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iterator>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <deque>
#include <map>
#include <cmath>

using namespace std;

typedef struct _instruction instruction;
struct _instruction {
	unsigned int address;
	bool branch;
};

class Perceptron{
private:
	int * weight_vector;
	int depth;
	int bias;
	int theta; // keeps from overtraining
	int prediction;
	int running_sum;

	friend class BranchPredictor;

public:
	Perceptron() {}
	Perceptron(int depth);
	~Perceptron();
	void predict(deque <int> global_history);
	void update(deque <int> global_history, int actual);
	void stats();
};

class BranchPredictor{
private:
	char **argv;
	vector<instruction> trace;
	ofstream output_file;

	// for perceptron branch predictor
	deque <int> global_history;
	map <unsigned int, Perceptron *> address_perceptron_map;


public:
	BranchPredictor(int argc, char* argv[]);
	~BranchPredictor();
	void read_file();
	vector<string> split_line(const string &str,
	char delim, vector<string> tokens);
	int dot_product(vector<int> a, vector<int> b);
	void simulate();
};

#endif