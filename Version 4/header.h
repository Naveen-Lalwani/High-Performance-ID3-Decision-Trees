#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <math.h>
#include <float.h>
#include <cstdlib>
#include <iomanip>
#include <immintrin.h>

using namespace std;
extern int featureCount;

typedef vector<string> vs;
typedef vector<vs> vvs;
typedef vector<int> vi;
typedef vector<vi> vvi;
typedef map<string, int> msi;
typedef vector<double> vd;
typedef map<string, int> columnHeader;
typedef map<int, msi> mimsi;
typedef map<int, int> mii;

struct node													// struct node defines the structure of a node of the decision tree
{
	int splitOn;											// Stores which attribute to split on at a particular node
	int label;											// Stores the class label for leaf nodes. For nodes that are not leaf nodes, it stores the value of the attribute of the parent's' split 
	bool isLeaf;											// boolean flag for leaf nodes
	vector<int> childrenValues;							// Stores the values of the childrens' attributes
	vector<node*> children;									// Stores pointers to the children of a node
};

void parse(string&, vvi&, mimsi&, int[]);									// Parses a single line from the input file and stores the information into a vector of vector of strings
void parseTest(string&, vvi&, mimsi&); 
void printAttributeTable(vvi&);								// For debugging purposes only. Prints a data table
vvs pruneTable(vvs&, string&, string);						// Prunes a table based on a column/attribute's name and the value of that attribute. Removes that column and all instances that have that value for that column
node* buildDecisionTree(vvi&, node*, int, int[]);					// Builds the decision tree based on the table it is passed
bool isHomogeneous(vvi&);									// Returns true if all instances in a subtable at a node have the same class label
vi countDistinct(vvi&, int, int);								// Returns a vector of integers containing the counts of all the various values of an attribute/column
int decideSplittingColumn(vvi&, int[]);							// Returns the column on which to split on. Decision of column is based on entropy
int returnColumnIndex(int&, vvi&);						// Returns the index of a column in a subtable
bool tableIsEmpty(vvi&);									// Returns true if a subtable is empty
void printDecisionTree(node*);								// For degubbing purposes only. Recursively prints decision tree
int testDataOnDecisionTree(vi&, node*, int);	// Runs a single instance of the test data through the decision tree. Returns the predicted class label
int returnIndexOfVector(vi&, int);						// Returns the index of a string in a vector of strings
double printPredictionsAndCalculateAccuracy(vi&, vi&);		// Outputs the predictions to file and returns the accuracy of the classification
vvi generateTableInfo(vvi &dataTable);						// Generates information about the table in a vector of vector of stings
void printTimeDetails();
int returnMostFrequentClass(vvi&, int);				// Returns the most frequent class from the training data. This class is used as the default class during the testing phase
bool contains(vvi&, int, int);
