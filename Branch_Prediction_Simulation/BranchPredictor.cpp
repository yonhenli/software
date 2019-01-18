#include "BranchPredictor.h"

BranchPredictor::BranchPredictor(int argc, char* argv[]) {
	if (argc != 2){
		throw range_error("command line arguments error");
	}

	this -> argv = argv;
	this -> output_file.open("output.txt", ios::out);
	this -> output_file.close();
}

BranchPredictor::~BranchPredictor() {
    // deallocate global history
    map <unsigned int, Perceptron *>::iterator itr;
    for (itr = address_perceptron_map.begin();
         itr != address_perceptron_map.end(); ++itr) {
        delete itr -> second;
    }
}

/**
 * this method process input file and parse address and branch result into a
 * struct and store in a vector.
 */
void BranchPredictor::read_file() {
	vector <string> tokens;
	string line;
	ifstream infile;

	/* temp variables */
	int temp_ctr = 0;
	unsigned int temp_addr;

	/* open the input file */
	infile.open(this -> argv[1]);
	if (!infile) {
		throw runtime_error("cannot open the file");
	}

	while (getline(infile, line) && temp_ctr < 30) {
		tokens = split_line(line, ' ', tokens);
		
		/* process address */
		temp_addr = strtoul(tokens.front().c_str(), NULL, 16);

		/* process target result */
		if (tokens.back() == "T") {
			this -> trace.push_back({temp_addr, true});
		}
		else if (tokens.back() == "NT") {
			this -> trace.push_back({temp_addr, false});
		}

		/* clear buffer */
		tokens.clear();
	}

	infile.close();
}

/**
 * This method parses a line by a specific delimiter.
 * @param	str		a string to be process
 * @param	delim	a delimiter
 * @param 	tokens	a vector to store each element
 * @return			a vector contains each element
 */
vector<string> BranchPredictor::split_line 
(const string &str, char delim, vector<string> tokens) {
    stringstream ss(str);
    string item;

    while (getline(ss, item, delim)) {
        tokens.push_back(item);
    }

    return tokens;
}

// int BranchPredictor::dot_product(vector<int> a, vector<int> b) {
//     if( a.size() != b.size() ) // error check
//     {
//         puts( "Error a's size not equal to b's size" ) ;
//         return -1 ;
//     }

//     // compute
//     int product = 0;
//     for (int i = 0; i < int(a.size()); i++)
//        product += (a[i])*(b[i]);
//     return product;
// }

void BranchPredictor::simulate() {
    read_file();
    
    // layers
    int layers = 8;
    // to show stats
    int num_pred = 0;
    // perceptron temp
    Perceptron * curr_perceptron;
    // actual branch output
    int actual = 0;

    deque <int> global_history;

    // initialize global history
    for (int i = 0; i < layers; i++) {
        this -> global_history.push_back(i);
    }

    vector <instruction> :: iterator itr;
    for (itr = this -> trace.begin(); itr != this -> trace.end(); ++itr){
        // if there is no such address in the map, then create a new
        // perceptron asscociated with it.
        if (this -> address_perceptron_map.count(itr -> address) == 0) {
            address_perceptron_map.insert(make_pair(itr -> address, new Perceptron(layers)));
        }

        // make prediction
        curr_perceptron = address_perceptron_map.find(itr -> address) -> second;
        curr_perceptron -> predict(this -> global_history);

        // get actual branch outcome
        if (itr -> branch == true) actual = 1;
        else actual = -1;

        // update perceptron
        curr_perceptron -> update(this -> global_history, actual);
        this -> global_history.push_front(actual);
        this -> global_history.pop_back();

        if (curr_perceptron -> prediction == actual) num_pred++;
    }

    cout << num_pred << ',' << this -> trace.size() << ';' << endl;
}

Perceptron::Perceptron(int depth){
    this -> depth = depth;
    this -> running_sum = 0;
    this -> prediction = 0;
    this -> bias = 0;
    this -> theta = 2 * depth + 14;
    this -> weight_vector = new int[depth];
    fill_n(this -> weight_vector, depth, 0);
}

Perceptron::~Perceptron(){
    delete[] this -> weight_vector;
}

void Perceptron::predict(deque <int> global_history) {
    this -> running_sum = this -> bias;
    for (int i = 0; i < this -> depth; i++) {
        this -> running_sum = this -> running_sum + global_history.at(i) *
        this -> weight_vector[i];
    }

    if ((this -> running_sum) < 0) {
        this -> prediction = -1;
    }
    else {
        this -> prediction = 1;
    }

}

void Perceptron::update(deque <int> global_history, int actual) {
    // original code
    // if (this -> prediction != actual || abs(this -> running_sum) < this -> theta) {
    //     this -> bias = this -> bias + (1 * actual);
    //     for(int i = 0; i < this -> depth; i ++) {
    //         this -> weight_vector[i] = this -> weight_vector[i] +
    //         (actual * global_history.at(i));
    //     }
    // }
    
    if ((abs(this -> running_sum) <= this -> theta) || this -> running_sum != actual) {
        this -> bias = this -> bias + (1 * actual);
        for (int i = 0; i < this -> depth; i++) {
            if (actual == global_history.at(i)) {
                this -> weight_vector[i] = this -> weight_vector[i] + 1;
            }
            else {
                this -> weight_vector[i] = this -> weight_vector[i] - 1;
            }
        }
    }
}

void Perceptron::stats() {
    cout << "bias: " << this -> bias << endl;
}