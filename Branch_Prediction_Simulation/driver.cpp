#include "BranchPredictor.h"

int main(int argc, char* argv[]) {
	BranchPredictor branch_predictor(argc, argv);
	branch_predictor.simulate();
	return 0;
}