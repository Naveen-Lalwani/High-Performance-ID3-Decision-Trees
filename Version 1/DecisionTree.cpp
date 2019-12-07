#include "fast_code_utils.h"
#include "header.h"


int main(int argc, const char *argv[])
{
	unsigned long long TRAIN_ST1;
	unsigned long long TRAIN_ET1;
	unsigned long long TEST_ST2;
	unsigned long long TEST_ET2;


	ifstream inputFile;												// Input file stream
	string singleInstance;											// Single line read from the input file 
	vvs dataTable;													// Input data in the form of a vector of vector of strings

	inputFile.open(argv[1]);
	if (!inputFile)													// If input file does not exist, print error and exit
	{
		cerr << "Error: Training data file not found!" << endl;
		exit(-1);
	}

	/* 
	 * Decision tree training phase
	 * In this phase, the training data is read
	 * from the file and stored into a vvs using
	 * the parse() function. The generateTableInfo()
	 * function extracts the attribute (column) names
	 * and also the values that each column can take.
	 * This information is also stored in a vvs.
	 * buildDecisionTree() function recursively
	 * builds trains the decision tree.
	 */
	while (getline(inputFile, singleInstance))						// Read from file, parse and store data
	{
		parse(singleInstance, dataTable);
	}
	inputFile.close(); 												// Close input file
	/*
	 * Table is read till here and all instances are recorded.
	 */
	
	vvs tableInfo = generateTableInfo(dataTable);					// Stores all the attributes and their values in a vector of vector of strings named tableInfo
	
	TRAIN_ST1 = rdtsc();
	node* root = new node;											// Declare and assign memory for the root node of the Decision Tree
	root -> isLeaf = false;
	root = buildDecisionTree(dataTable, root, tableInfo, 0);			// Recursively build and train decision tree
	TRAIN_ET1 = rdtsc() - TRAIN_ST1;

	string defaultClass = returnMostFrequentClass(dataTable);		// Stores the most frequent class in the training data. This is used as the default class label
	dataTable.clear(); 												// clear dataTable of training data to store testing data


	/*
	 * Decision tree testing phase
	 * In this phase, the testing is read
	 * from the file, parsed and stored.
	 * Each row in the table is made to
	 * traverse down the decision tree
	 * till a class label is found.
	 */
	inputFile.clear();
	inputFile.open(argv[2]); 										// Open test file
	if (!inputFile) 												// Exit if test file is not found
	{
		cerr << "Error: Testing data file not found!" << endl;
		exit(-1);
	}
	
	while (getline(inputFile, singleInstance)) 						// Store test data in a table
	{
		parse(singleInstance, dataTable);
	}
	
	
	vs predictedClassLabels;										// Stores the predicted class labels for each row
	vs givenClassLabels;											// Stores the given class labels in the test data
	
	for (int iii = 1; iii < dataTable.size(); iii++)				// Store given class labels in vector of strings named givenClassLabels
	{
		string data = dataTable[iii][dataTable[0].size()-1];
		givenClassLabels.push_back(data);
	}
	
	TEST_ST2 = rdtsc();
	for (int iii = 1; iii < dataTable.size(); iii++)				// Predict class labels based on the decision tree
	{
		string someString = testDataOnDecisionTree(dataTable[iii], root, tableInfo, defaultClass);
		predictedClassLabels.push_back(someString);
	}
	TEST_ET2 = rdtsc() - TEST_ST2;

	dataTable.clear();

	/* Print output */
	//printDecisionTree(root);
	//ofstream outputFile;
	//outputFile.open("decisionTreeOutput.txt", ios::app);
	//outputFile << endl << "--------------------------------------------------" << endl;
	double accuracy = printPredictionsAndCalculateAccuracy(givenClassLabels, predictedClassLabels);			// calculate accuracy of classification
	cout << endl << endl <<"Accuracy of decision tree classifier = " << accuracy << "%"; 
	//outputFile << "Accuracy of decision tree classifier = " << accuracy << "%"; 							// Print out accuracy to console


	cout << "Cycles for Training the tree = " << TRAIN_ET1 << endl;
	cout << "Cycles for Testing the tree = " << TEST_ET2 << endl;
	printTimeDetails();

	return 0;
}
