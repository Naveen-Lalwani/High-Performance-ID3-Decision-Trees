#include "fast_code_utils.h"
#include "header.h"
#include <string.h>

int featureCount = 0;
int main(int argc, const char *argv[])
{	
	unsigned long long TRAIN_ST1;
	unsigned long long TRAIN_ET1;
	unsigned long long TEST_ST2;
	unsigned long long TEST_ET2;

	static const int FEATURE_IDENTIFIER = 500; 
	
	ifstream inputFile;												// Input file stream
	string singleInstance;											// Single line read from the input file 
														
	vvi intEncodedTable;											// Input data in the form of a vector of vector of strings
	mimsi uniqueValsPerCol;											// Hashmap from Integer (Column Encoded) to Hash Map of String(Attribute Value): Integer Encode
	
	inputFile.open(argv[1]);
	if (!inputFile)	{												// If input file does not exist, print error and exit
		
		cerr << "Error: Training data file not found!" << endl;
		exit(-1);
	}

	/*
	 * The first row is taken and column names are stored in a hash map
	 * so that we can easily access the unique values in the columns/attributes
	 * through the hash map that maps unique string values to its integer code.
	 */	 
	getline(inputFile, singleInstance);
	
	columnHeader topMap;
	
	vi temp;
	
	
	// Get the mapping from column name to column integer encoding 
	while (singleInstance.length() != 0 && singleInstance.find(',') != string::npos) {
		size_t pos;
		string singleAttribute;
		
		pos = singleInstance.find_first_of(',');				// , position
		singleAttribute = singleInstance.substr(0, pos);		// take attribute from 0 to , position
 
		int colIdentifier = featureCount + FEATURE_IDENTIFIER;
		temp.push_back(colIdentifier);
		topMap[singleAttribute] = colIdentifier;					// Add the attribute to hashmap a mapping from String attribute to its integer code
		uniqueValsPerCol[colIdentifier];
			
		singleInstance.erase(0, pos+1);							// Move onto the next attribute
		featureCount++;
	}
	
	topMap[singleInstance] = featureCount + FEATURE_IDENTIFIER;
	uniqueValsPerCol[featureCount + FEATURE_IDENTIFIER];
	temp.push_back(featureCount + FEATURE_IDENTIFIER);
	intEncodedTable.push_back(temp); 
	
	/*	
	for(map<string, vs>::const_iterator it = top.begin(); it != top.end(); ++it) {
    	std::cout << it->first << " " << "\n";
	}
	
	for (int i =0; i < topRow.size(); i++) {
		std::cout <<topRow[i] << "\n";
	}
	*/
	
	/*
	 * This was the reason of the value looking like integer overflow.
	 */
	int encode[++featureCount];
	memset(encode, 0, featureCount*sizeof(int));				 
	
	while (getline(inputFile, singleInstance))						// Read from file, parse and store data
	{
		parse(singleInstance, intEncodedTable, uniqueValsPerCol, encode);
	}
	inputFile.close(); 												// Close input file

	temp.clear();
	singleInstance.clear();
	//printAttributeTable(intEncodedTable);
	
	int defaultClass = returnMostFrequentClass(intEncodedTable, encode[featureCount - 1]);		// Stores the most frequent class in the training data. This is used as the default class label

	TRAIN_ST1 = rdtsc();
	node* root = new node;											// Declare and assign memory for the root node of the Decision Tree
	root->isLeaf = false;
	root = buildDecisionTree(intEncodedTable, root, 0, encode);			// Recursively build and train decision tree
	TRAIN_ET1 = rdtsc() - TRAIN_ST1;

	intEncodedTable.clear();	

	/*
	 * Decision tree testing phase
	 * In this phase, the testing is read
	 * from the file, parsed and stored.
	 * Each row in the table is made to
	 * traverse down the decision tree
	 * till a class label is found.
	 */
	 
	vvi testIntEncodedTable;
	 
	inputFile.clear();
	inputFile.open(argv[2]); 										// Open test file
	if (!inputFile) 												// Exit if test file is not found
	{
		cerr << "Error: Testing data file not found!" << endl;
		exit(-1);
	}
	
	getline(inputFile, singleInstance);
	
	
	while (singleInstance.length() != 0 && singleInstance.find(',') != string::npos) {
		size_t pos;
		string singleAttribute;
		
		pos = singleInstance.find_first_of(',');				// , position
		singleAttribute = singleInstance.substr(0, pos);		// take attribute from 0 to , position
 	
		temp.push_back(topMap[singleAttribute]);
			
		singleInstance.erase(0, pos+1);							// Move onto the next attribute
	}
	
	temp.push_back(topMap[singleInstance]);
	testIntEncodedTable.push_back(temp);
	
	while (getline(inputFile, singleInstance)) 						// Store test data in a table
	{
		parseTest(singleInstance, testIntEncodedTable, uniqueValsPerCol);
	}
	//printAttributeTable(testIntEncodedTable);
	
	
	vi predictedClassLabels;										// Stores the predicted class labels for each row
	vi givenClassLabels;											// Stores the given class labels in the test data
	for (int iii = 1; iii < testIntEncodedTable.size(); iii++)				// Store given class labels in vector of strings named givenClassLabels
	{
		int data = testIntEncodedTable[iii][testIntEncodedTable[0].size() - 1];
		givenClassLabels.push_back(data);
	}
	
	TEST_ST2 = rdtsc();
	for (int iii = 1; iii < testIntEncodedTable.size(); iii++)				// Predict class labels based on the decision tree
	{
		int predictedLabel = testDataOnDecisionTree(testIntEncodedTable[iii], root, defaultClass);
		predictedClassLabels.push_back(predictedLabel);
	}
	TEST_ET2 = rdtsc() - TEST_ST2;

	testIntEncodedTable.clear();

	/* Print output */
	//printDecisionTree(root);
	//ofstream outputFile;
	//outputFile.open("decisionTreeOutput.txt", ios::app);
	//outputFile << endl << "--------------------------------------------------" << endl;
	double accuracy = printPredictionsAndCalculateAccuracy(givenClassLabels, predictedClassLabels);			// calculate accuracy of classification
	cout << endl  << endl <<"Accuracy of decision tree classifier = " << accuracy << "%" << endl; 
	//outputFile << "Accuracy of decision tree classifier = " << accuracy << "%"; 							// Print out accuracy to console
	
	uniqueValsPerCol.clear();
	topMap.clear();
	temp.clear();


	cout << "Cycles for Training the tree = " << TRAIN_ET1 << endl;
	cout << "Cycles for Testing the tree = " << TEST_ET2 << endl;
	printTimeDetails();
	
	return 0;
}