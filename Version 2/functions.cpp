#include "fast_code_utils.h"
#include "header.h"

static const int MAX_DEPTH = 1;
static const int FEATURE_IDENTIFIER = 500;

unsigned long long PRUNE_TABLE_ST;
unsigned long long PRUNE_TABLE_ET;
unsigned long long PRUNE_TABLE_COUNT;

unsigned long long IS_HOMOGENEOUS_ST;
unsigned long long IS_HOMOGENEOUS_ET;
unsigned long long IS_HOMOGENEOUS_COUNT;

unsigned long long COUNT_DISTINCT_ST;
unsigned long long COUNT_DISTINCT_ET;
unsigned long long COUNT_DISTINCT_COUNT;

unsigned long long DECIDE_SPLITTING_COLUMN_ST;
unsigned long long DECIDE_SPLITTING_COLUMN_ET;
unsigned long long DECIDE_SPLITTING_COLUMN_COUNT;

unsigned long long RETURN_COLUMN_INDEX_ST;
unsigned long long RETURN_COLUMN_INDEX_ET;
unsigned long long RETURN_COLUMN_INDEX_COUNT;

unsigned long long RETURN_INDEX_OF_VECTOR_ST;
unsigned long long RETURN_INDEX_OF_VECTOR_ET;
unsigned long long RETURN_INDEX_OF_VECTOR_COUNT;

unsigned long long GENERATE_TABLE_INFO_ST;
unsigned long long GENERATE_TABLE_INFO_ET;

unsigned long long RETURN_MOST_FREQUENT_CLASS_ST;
unsigned long long RETURN_MOST_FREQUENT_CLASS_ET;
unsigned long long RETURN_MOST_FREQUENT_CLASS_COUNT;

unsigned long long BUILD_DECISION_TREE_ST;
unsigned long long BUILD_DECISION_TREE_ET;
unsigned long long BUILD_DECISION_TREE_COUNT;


unsigned long long PRUNE_TABLE;
unsigned long long IS_HOMOGENEOUS;
unsigned long long COUNT_DISTINCT;
unsigned long long DECIDE_SPLITTING_COLUMN;
unsigned long long RETURN_COLUMN_INDEX;
unsigned long long RETURN_INDEX_OF_VECTOR;
unsigned long long GENERATE_TABLE_INFO;
unsigned long long RETURN_MOST_FREQUENT_CLASS;
unsigned long long BUILD_DECISION_TREE;


/* 
 * Parses a string and stores data
 * into a vector of vector of int
 */
void parse(string &someString, vvi &attributeTable, mimsi &uniqueValsPerCol , int encode[])
{
	int colCount = 0;
	vi vectorOfInts;
	
	while (someString.length() != 0 && someString.find(',') != string::npos) {
		
		size_t pos;
		string singleAttribute;
		
		pos = someString.find_first_of(',');
		singleAttribute = someString.substr(0, pos);
		
		if (uniqueValsPerCol[FEATURE_IDENTIFIER + colCount].find(singleAttribute) == (uniqueValsPerCol[FEATURE_IDENTIFIER + colCount].end())) { // If unique value doesn't exist in the map, add it.
			uniqueValsPerCol[FEATURE_IDENTIFIER + colCount][singleAttribute] = encode[colCount];
			encode[colCount]++;			// Incremnet value of encode at that column to keep track of number of unique value that needs to exist for next new value										
		} 
		
		vectorOfInts.push_back(uniqueValsPerCol[FEATURE_IDENTIFIER + colCount][singleAttribute]);
		someString.erase(0, pos+1);
		colCount++;
	}
	
	if (uniqueValsPerCol[FEATURE_IDENTIFIER + colCount].find(someString) == (uniqueValsPerCol[FEATURE_IDENTIFIER + colCount].end())) { // If unique value doesn't exist in the map, add it.
		uniqueValsPerCol[FEATURE_IDENTIFIER + colCount][someString] = encode[colCount];
		encode[colCount]++;			// Incremnet value of encode at that column to keep track of number of unique value that needs to exist for next new value										
	}
	 
	vectorOfInts.push_back(uniqueValsPerCol[FEATURE_IDENTIFIER + colCount][someString]);
	attributeTable.push_back(vectorOfInts);
	vectorOfInts.clear();
}


/* 
 * Parses a string from test data and stores data
 * into a vector of vector of ints
 */
void parseTest(string &someString, vvi &attributeTable, mimsi &uniqueValsPerCol)
{
	int colCount = 0;
	vi vectorOfInts;
	
	while (someString.length() != 0 && someString.find(',') != string::npos) {
		
		size_t pos;
		string singleAttribute;
		
		pos = someString.find_first_of(',');
		singleAttribute = someString.substr(0, pos); 
		
		vectorOfInts.push_back(uniqueValsPerCol[FEATURE_IDENTIFIER + colCount][singleAttribute]);
		someString.erase(0, pos+1);
		colCount++;
	}
	 
	vectorOfInts.push_back(uniqueValsPerCol[FEATURE_IDENTIFIER + colCount][someString]);
	attributeTable.push_back(vectorOfInts);
	vectorOfInts.clear();
}

/*
 * Prints a vector of vector of strings
 * For debugging purposes only.
 */
void printAttributeTable(vvi &attributeTable)
{	
	FILE *f = fopen("trainDataset.txt", "w");
	int inner, outer;
	for (outer = 0; outer < attributeTable.size(); outer++) {
		for (inner = 0; inner < attributeTable[outer].size(); inner++) {
			fprintf(f, "%d \t", attributeTable[outer][inner]);
		}
		fprintf(f, "\n");
	}
	fprintf(f, "\n");
}

/*
 * Prunes a table based on a column/attribute's name
 * and value of that attribute. Removes that column
 * and all rows that have that value for that column.
 */
vvi pruneTable(vvi &attributeTable, int &colName, int value)
{
	PRUNE_TABLE_ST = rdtsc();
	int iii, jjj;
	vvi prunedTable;
	int column = -1;
	vi headerRow;
	for (iii = 0; iii < attributeTable[0].size(); iii++) {
		if (attributeTable[0][iii] == colName) {
			column = iii;
			break;
		}
	}
	for (iii = 0; iii < attributeTable[0].size(); iii++) {
		 if (iii != column) {
		 	headerRow.push_back(attributeTable[0][iii]);
		 }
	}
	prunedTable.push_back(headerRow);
	for (iii = 0; iii < attributeTable.size(); iii++) {
		vi auxRow;
		if (attributeTable[iii][column] == value) {
			for (jjj = 0; jjj < attributeTable[iii].size(); jjj++) {
				if(jjj != column) {
					auxRow.push_back(attributeTable[iii][jjj]);
				}
			}
			prunedTable.push_back(auxRow);
		}
	}

	PRUNE_TABLE_ET = rdtsc() - PRUNE_TABLE_ST;
	PRUNE_TABLE = PRUNE_TABLE + PRUNE_TABLE_ET;
	PRUNE_TABLE_COUNT++;

	return prunedTable;
}

/*
 * Recursively builds the decision tree based on
 * the data that it is passed and tha table info.
 */
node* buildDecisionTree(vvi &table, node* nodePtr, int depth, int encoded[])
{	
	BUILD_DECISION_TREE_ST = rdtsc();

	if (tableIsEmpty(table)) {
		BUILD_DECISION_TREE_ET = rdtsc() - BUILD_DECISION_TREE_ST;
		BUILD_DECISION_TREE_COUNT++;
		BUILD_DECISION_TREE += BUILD_DECISION_TREE_ET;
		return NULL;
	}
	if (depth == MAX_DEPTH) {
		nodePtr->isLeaf = true;
		nodePtr->label = returnMostFrequentClass(table);
		BUILD_DECISION_TREE_ET = rdtsc() - BUILD_DECISION_TREE_ST;
		BUILD_DECISION_TREE_COUNT++;
		BUILD_DECISION_TREE += BUILD_DECISION_TREE_ET;
		return nodePtr;
	}
	if (isHomogeneous(table)) {
		nodePtr->isLeaf = true;
		nodePtr->label = table[1][table[1].size()-1];
		BUILD_DECISION_TREE_ET = rdtsc() - BUILD_DECISION_TREE_ST;
		BUILD_DECISION_TREE_COUNT++;
		BUILD_DECISION_TREE += BUILD_DECISION_TREE_ET;
		return nodePtr;
	} else {
		int splittingCol = decideSplittingColumn(table);
		nodePtr->splitOn = splittingCol;
		int colIndex = splittingCol - FEATURE_IDENTIFIER;
		int iii;
		for (iii = 0; iii < encoded[colIndex]; iii++) {
			node* newNode = (node*) new node;
			newNode->label = iii;
			nodePtr->childrenValues.push_back(iii);
			newNode->isLeaf = false;
			newNode->splitOn = splittingCol;
			vvi auxTable = pruneTable(table, splittingCol, iii);
			nodePtr->children.push_back(buildDecisionTree(auxTable, newNode, depth + 1, encoded));
		}
	}
	BUILD_DECISION_TREE_ET = rdtsc() - BUILD_DECISION_TREE_ST;
	BUILD_DECISION_TREE_COUNT++;
	BUILD_DECISION_TREE += BUILD_DECISION_TREE_ET;
	return nodePtr;
}

/*
 * Returns true if all rows in a subtable
 * have the same class label.
 * This means that that node's class label
 * has been decided.
 */
bool isHomogeneous(vvi &table)
{
	IS_HOMOGENEOUS_ST = rdtsc();


	int lastCol = table[0].size() - 1;
	int firstValue = table[1][lastCol];
	for (int i = 1; i < table.size(); i++) {
		if (firstValue != table[i][lastCol]) {
			IS_HOMOGENEOUS_ET = rdtsc() - IS_HOMOGENEOUS_ST;
			IS_HOMOGENEOUS_COUNT++;
			IS_HOMOGENEOUS += IS_HOMOGENEOUS_ET;

			return false;
		}
	}
	IS_HOMOGENEOUS_ET = rdtsc() - IS_HOMOGENEOUS_ST;
	IS_HOMOGENEOUS_COUNT++;
	IS_HOMOGENEOUS += IS_HOMOGENEOUS_ET;
	return true;
}

/*
 * Returns a vector of integers containing the counts
 * of all the various values of an attribute/column.
 */
vi countDistinct(vvi &table, int column)
{
	COUNT_DISTINCT_ST = rdtsc();

	vi vectorOfInts;
	vi counts;
	
	bool found = false;
	int foundIndex;
	for (int iii = 1; iii < table.size(); iii++) {
		for (int jjj = 0; jjj < vectorOfInts.size(); jjj++) {
			if (vectorOfInts[jjj] == table[iii][column]) {
				found = true;
				foundIndex = jjj;
				break;
			} else {
				found = false;
			}
		}
		if (!found) {
			counts.push_back(1);
			vectorOfInts.push_back(table[iii][column]);
		} else {
			counts[foundIndex]++;
		}
	}
	int sum = 0;
	for (int iii = 0; iii < counts.size(); iii++) {
		sum += counts[iii];
	}
	counts.push_back(sum);

	COUNT_DISTINCT_ET = rdtsc() - COUNT_DISTINCT_ST;
	COUNT_DISTINCT += COUNT_DISTINCT_ET;
	COUNT_DISTINCT_COUNT++;

	return counts;
}

/*
 * Decides which column to split on
 * based on entropy. Returns the column
 * with the least entropy.
 */
int decideSplittingColumn(vvi &table)
{
	DECIDE_SPLITTING_COLUMN_ST = rdtsc();


	double marginalEntropy = 0.0;
	vi output = countDistinct(table, table[0].size() - 1);
	for (int j = 0; j < output.size() - 1; j++) {
		double temp = (double) output[j];
		marginalEntropy -= (temp / output[output.size() - 1])*(log(temp/output[output.size()-1]) / log(2));
	}
	//cout << marginalEntropy << endl;
	double informationGain = -1;
	int splittingColumn = 0;
	vi entropies;
	for (int column = 0; column < table[0].size() - 1; column++) {
		int colName = table[0][column];
		mii tempMap;
		vi counts = countDistinct(table, column);
		vd attributeEntropy;
		double conditionalEntropy = 0.0;
		for (int iii = 1; iii < table.size(); iii++) {
			double entropy = 0.0;
			if (tempMap.find(table[iii][column]) != tempMap.end()) { 	// IF ATTRIBUTE IS ALREADY FOUND IN A COLUMN, UPDATE IT'S FREQUENCY
				tempMap[table[iii][column]]++;
			} else { 							// IF ATTRIBUTE IS FOUND FOR THE FIRST TIME IN A COLUMN, THEN PROCESS IT AND CALCULATE IT'S ENTROPY
				tempMap[table[iii][column]] = 1;
				vvi tempTable = pruneTable(table, colName, table[iii][column]);
				vi classCounts = countDistinct(tempTable, tempTable[0].size()-1);
				for (int jjj = 0; jjj < classCounts.size() - 1; jjj++) {
					double temp = (double) classCounts[jjj];
					entropy -= (temp/classCounts[classCounts.size()-1])*(log(temp/classCounts[classCounts.size()-1]) / log(2));
				}
				attributeEntropy.push_back(entropy);
				entropy = 0.0;
			}
		}
		for (int iii = 0; iii < counts.size() - 1; iii++) {
			conditionalEntropy += ((double) counts[iii] * (double) attributeEntropy[iii]);
		}
		conditionalEntropy = conditionalEntropy / ((double) counts[counts.size() - 1]);
		
		if ((marginalEntropy - conditionalEntropy) > informationGain) {
			informationGain = marginalEntropy - conditionalEntropy;
			splittingColumn = column;
		}
	}


	DECIDE_SPLITTING_COLUMN_ET = rdtsc() - DECIDE_SPLITTING_COLUMN_ST;
	DECIDE_SPLITTING_COLUMN_COUNT++;
	DECIDE_SPLITTING_COLUMN += DECIDE_SPLITTING_COLUMN_ET;

	return table[0][splittingColumn];
}


/*
 * Returns true if the table is empty
 * returns false otherwise
 */
bool tableIsEmpty(vvi &table) {
	return (table.size() == 1);
}

/*
 * Recursively prints the decision tree
 * For debugging purposes only
 */
void printDecisionTree(node* nodePtr)
{
	if(nodePtr == NULL) {
		return;
	}
	if (!nodePtr->children.empty()) {
		cout << " Value: " << nodePtr->label << endl;
		cout << "Split on: " << nodePtr->splitOn;
		int iii;
		for (iii = 0; iii < nodePtr->children.size(); iii++) {   
			cout << "\t";
			printDecisionTree(nodePtr->children[iii]);
		}
		return;
        } else {
		cout << "Predicted class = " << nodePtr->label;
		return;
	}
}

/*
 * Takes a row and traverses that row through
 * the decision tree to find out the 
 * predicted class label. If none is found
 * returns the default class label which is
 * the class label with the highest frequency.
 */
int testDataOnDecisionTree(vi &singleLine, node* nodePtr, int defaultClass)
{
	int prediction;
	while (!nodePtr->isLeaf && !nodePtr->children.empty()) {
		int index = nodePtr->splitOn - FEATURE_IDENTIFIER;
		int value = singleLine[index];
		int childIndex = returnIndexOfVector(nodePtr->childrenValues, value);
		nodePtr = nodePtr->children[childIndex];
		if (nodePtr == NULL) {
			prediction = defaultClass;
			break;
		} else {
			prediction = nodePtr->label;
		}
	}
	return prediction;
}

/*
 * Returns an integer which is the index
 * of a string in a vector of strings
 */
int returnIndexOfVector(vi &stringVector, int value)
{
	RETURN_INDEX_OF_VECTOR_ST = rdtsc();

	int iii;
	for (iii = 0; iii < stringVector.size(); iii++) {
		if (stringVector[iii] == value)	{
			RETURN_INDEX_OF_VECTOR_ET = rdtsc() - RETURN_INDEX_OF_VECTOR_ST;
			RETURN_INDEX_OF_VECTOR += RETURN_INDEX_OF_VECTOR_ET;
			RETURN_INDEX_OF_VECTOR_COUNT++;
			return iii;
		}
	}

	RETURN_INDEX_OF_VECTOR_ET = rdtsc() - RETURN_INDEX_OF_VECTOR_ST;
	RETURN_INDEX_OF_VECTOR += RETURN_INDEX_OF_VECTOR_ET;
	RETURN_INDEX_OF_VECTOR_COUNT++;
	return -1;
}

/*
 * Outputs the predictions to file
 * and returns the accuracy of the classification
 */
double printPredictionsAndCalculateAccuracy(vi &givenData, vi &predictions)
{
	ofstream outputFile;
	outputFile.open("decisionTreeOutput.txt");
	int correct = 0;
	outputFile << setw(3) << "#" << setw(16) << "Given Class" << setw(31) << right << "Predicted Class" << endl;
	outputFile << "--------------------------------------------------" << endl;
	for (int iii = 0; iii < givenData.size(); iii++) {
		outputFile << setw(3) << iii+1 << setw(16) << givenData[iii];
		if (givenData[iii] == predictions[iii]) {
			correct++;
			outputFile << "  ------------  ";
		} else {
			outputFile << "  xxxxxxxxxxxx  ";
		}
		outputFile << predictions[iii] << endl;
	}
	outputFile << "--------------------------------------------------" << endl;
	outputFile << "Total number of instances in test data = " << givenData.size() << endl;
	outputFile << "Number of correctly predicted instances = " << correct << endl;
	outputFile.close();
	return (double) correct/givenData.size() * 100;
}


/*
 * Returns the most frequent class from the training data
 * This class will be used as the default class label
 */
int returnMostFrequentClass(vvi &dataTable)
{
	RETURN_MOST_FREQUENT_CLASS_ST = rdtsc();

	mii trainingClasses;           													 // Stores the classlabels and their frequency
	for (int i = 1; i < dataTable.size(); i++) {
		if (trainingClasses.count(dataTable[i][dataTable[0].size() - 1]) == 0) {
			trainingClasses[dataTable[i][dataTable[0].size() - 1]] = 1;
		} else {
			trainingClasses[dataTable[i][dataTable[0].size()-1]]++;
		}
	}   
	
	
	mii::iterator mapIter;
	int highestClassCount = 0;
	int mostFrequentClass;
	for (mapIter = trainingClasses.begin(); mapIter != trainingClasses.end(); mapIter++) {
		if (mapIter->second >= highestClassCount) {
			highestClassCount = mapIter->second;
			mostFrequentClass = mapIter->first;
		}   
	}

	RETURN_MOST_FREQUENT_CLASS_ET = rdtsc() - RETURN_MOST_FREQUENT_CLASS_ST;
	RETURN_MOST_FREQUENT_CLASS_COUNT++;
	RETURN_MOST_FREQUENT_CLASS += RETURN_MOST_FREQUENT_CLASS_ET;

	return mostFrequentClass;
}

void printTimeDetails() {
	cout << "Prune Table = " << PRUNE_TABLE / PRUNE_TABLE_COUNT << endl;
	cout << "Is Homogeneous = " << IS_HOMOGENEOUS << endl;
	cout << "Count Distinct = " << COUNT_DISTINCT / COUNT_DISTINCT_COUNT  << endl;
	cout << "Decide Splitting Column = " << DECIDE_SPLITTING_COLUMN << endl; 
	cout << "Return Index of Vector = " << RETURN_INDEX_OF_VECTOR / RETURN_INDEX_OF_VECTOR_COUNT << endl;
	cout << "Return Most Frequent Class = " << RETURN_MOST_FREQUENT_CLASS / RETURN_MOST_FREQUENT_CLASS_COUNT << endl;
	cout << "Build Decision Tree = " << BUILD_DECISION_TREE / BUILD_DECISION_TREE_COUNT << endl;
}
