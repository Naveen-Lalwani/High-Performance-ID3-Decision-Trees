#include "fast_code_utils.h"
#include "header.h"


int MAX_DEPTH = 1;

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


unsigned long long PRUNE_TABLE;
unsigned long long IS_HOMOGENEOUS;
unsigned long long COUNT_DISTINCT;
unsigned long long DECIDE_SPLITTING_COLUMN;
unsigned long long RETURN_COLUMN_INDEX;
unsigned long long RETURN_INDEX_OF_VECTOR;
unsigned long long GENERATE_TABLE_INFO;
unsigned long long RETURN_MOST_FREQUENT_CLASS;

bool firstCall1 = true;
bool firstCall2 = true;



/* 
 * Parses a string and stores data
 * into a vector of vector of strings
 */
void parse(string& someString, vvs &attributeTable)
{
	int attributeCount = 0;
	vs vectorOfStrings;
	while (someString.length() != 0 && someString.find(',') != string::npos)
	{
		size_t pos;
		string singleAttribute;
		pos = someString.find_first_of(',');
		singleAttribute = someString.substr(0, pos);
		vectorOfStrings.push_back(singleAttribute);
		someString.erase(0, pos+1);
	}
	vectorOfStrings.push_back(someString);
	attributeTable.push_back(vectorOfStrings);
	vectorOfStrings.clear();
}

/*
 * Prints a vector of vector of strings
 * For debugging purposes only.
 */
void printAttributeTable(vvs &attributeTable)
{
	int inner, outer;
	for (outer = 0; outer < attributeTable.size(); outer++) {
		for (inner = 0; inner < attributeTable[outer].size(); inner++) {
			cout << attributeTable[outer][inner] << "\t";
		}
		cout << endl;
	}
}

/*
 * Prunes a table based on a column/attribute's name
 * and value of that attribute. Removes that column
 * and all rows that have that value for that column.
 */
vvs pruneTable(vvs &attributeTable, string &colName, string value)
{	PRUNE_TABLE_ST = rdtsc();

	int iii, jjj;
	vvs prunedTable;
	int column = -1;
	vs headerRow;
	for (iii = 0; iii < attributeTable[0].size() - 1; iii++) {
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
	for (iii = 1; iii < attributeTable.size(); iii++) {
		vs auxRow;
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
node* buildDecisionTree(vvs &table, node* nodePtr, vvs &tableInfo, int depth)
{
	if (tableIsEmpty(table)) {
		return NULL;
	}
	if (depth == MAX_DEPTH) {
		nodePtr->isLeaf = true;
		nodePtr->label = returnMostFrequentClass(table);
		return nodePtr;
	}
	if (isHomogeneous(table)) {
		nodePtr->isLeaf = true;
		nodePtr->label = table[1][table[1].size() - 1];
		return nodePtr;
	} else {
		string splittingCol = decideSplittingColumn(table);
		nodePtr->splitOn = splittingCol;
		int colIndex = returnColumnIndex(splittingCol, tableInfo);
		int iii;
		for (iii = 0; iii < tableInfo[colIndex].size(); iii++) {
			node* newNode = (node*) new node;
			newNode->label = tableInfo[colIndex][iii];
			nodePtr->childrenValues.push_back(tableInfo[colIndex][iii]);
			newNode->isLeaf = false;
			newNode->splitOn = splittingCol;
			vvs auxTable = pruneTable(table, splittingCol, tableInfo[colIndex][iii]);
			nodePtr->children.push_back(buildDecisionTree(auxTable, newNode, tableInfo, depth + 1));
		}
	}
	return nodePtr;
}

/*
 * Returns true if all rows in a subtable
 * have the same class label.
 * This means that that node's class label
 * has been decided.
 */
bool isHomogeneous(vvs &table) {

	IS_HOMOGENEOUS_ST = rdtsc();

	int iii;
	int lastCol = table[0].size() - 1;
	string firstValue = table[1][lastCol];
	for (iii = 1; iii < table.size(); iii++) {
		if (firstValue != table[iii][lastCol]) {
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
vi countDistinct(vvs &table, int column)
{
	COUNT_DISTINCT_ST = rdtsc();

	vs vectorOfStrings;
	vi counts;
	bool found = false;
	int foundIndex;
	for (int iii = 1; iii < table.size(); iii++) {
		for (int jjj = 0; jjj < vectorOfStrings.size(); jjj++) {
			if (vectorOfStrings[jjj] == table[iii][column]) {
				found = true;
				foundIndex = jjj;
				break;
			} else {
				found = false;
			}
		}
		if (!found) {
			counts.push_back(1);
			vectorOfStrings.push_back(table[iii][column]);
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
string decideSplittingColumn(vvs &table)
{
	DECIDE_SPLITTING_COLUMN_ST = rdtsc();

	// Calculation of Marginal Entropy
	double marginalEntropy = 0.0;
	vi output = countDistinct(table, table[0].size() - 1);
	for (int j = 0; j < output.size() - 1; j++) {
		double temp = (double) output[j];
		marginalEntropy -= (temp / output[output.size() - 1])*(log(temp/output[output.size()-1]) / log(2));
	}
	
	int column, iii;
	double infromnationGain = -1;
	int splittingColumn = 0;
	vi entropies;
	for (column = 0; column < table[0].size() - 1; column++) {
		string colName = table[0][column];
		msi tempMap;
		vi counts = countDistinct(table, column);
		vd attributeEntropy;
		double conditionalEntropy = 0.0;
		for (iii = 1; iii < table.size(); iii++) {
			double entropy = 0.0;
			if (tempMap.find(table[iii][column]) != tempMap.end()) { 	// IF ATTRIBUTE IS ALREADY FOUND IN A COLUMN, UPDATE IT'S FREQUENCY
				tempMap[table[iii][column]]++;
			} else { 							// IF ATTRIBUTE IS FOUND FOR THE FIRST TIME IN A COLUMN, THEN PROCESS IT AND CALCULATE IT'S ENTROPY
				tempMap[table[iii][column]] = 1;
				vvs tempTable = pruneTable(table, colName, table[iii][column]);
				vi classCounts = countDistinct(tempTable, tempTable[0].size()-1);
				int jjj, kkk;
				for (jjj = 0; jjj < classCounts.size() - 1; jjj++) {
					double temp = (double) classCounts[jjj];
					entropy -= (temp/classCounts[classCounts.size()-1])*(log(temp/classCounts[classCounts.size()-1]) / log(2));
				}
				attributeEntropy.push_back(entropy);
				entropy = 0.0;
			}
		}
		for (iii = 0; iii < counts.size() - 1; iii++) {
			conditionalEntropy += ((double) counts[iii] * (double) attributeEntropy[iii]);
		}
		conditionalEntropy = conditionalEntropy / ((double) counts[counts.size() - 1]);
		
		if ((marginalEntropy - conditionalEntropy) > infromnationGain) {
			infromnationGain = marginalEntropy - conditionalEntropy;
			splittingColumn = column;
		}
	}
	
	DECIDE_SPLITTING_COLUMN_ET = rdtsc() - DECIDE_SPLITTING_COLUMN_ST;
	DECIDE_SPLITTING_COLUMN_COUNT++;
	DECIDE_SPLITTING_COLUMN += DECIDE_SPLITTING_COLUMN_ET;

	return table[0][splittingColumn];
}

/*
 * Returns an integer which is the
 * index of a column passed as a string
 */
int returnColumnIndex(string &columnName, vvs &tableInfo)
{
	RETURN_COLUMN_INDEX_ST = rdtsc();
	int iii;
	for (iii = 0; iii < tableInfo.size(); iii++) {
		if (tableInfo[iii][0] == columnName) {
			RETURN_COLUMN_INDEX_ET = rdtsc() - RETURN_COLUMN_INDEX_ST;
			RETURN_COLUMN_INDEX_COUNT++;
			RETURN_COLUMN_INDEX += RETURN_COLUMN_INDEX_ET;
			return iii;
		}
	}
	RETURN_COLUMN_INDEX_ET = rdtsc() - RETURN_COLUMN_INDEX_ST;
	RETURN_COLUMN_INDEX += RETURN_COLUMN_INDEX_ET;
	RETURN_COLUMN_INDEX_COUNT++;
	return -1;
}

/*
 * Returns true if the table is empty
 * returns false otherwise
 */
bool tableIsEmpty(vvs &table)
{
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
string testDataOnDecisionTree(vs &singleLine, node* nodePtr, vvs &tableInfo, string defaultClass)
{
	string prediction;
	while (!nodePtr->isLeaf && !nodePtr->children.empty()) {
		int index = returnColumnIndex(nodePtr->splitOn, tableInfo);
		string value = singleLine[index];
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
int returnIndexOfVector(vs &stringVector, string value)
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
double printPredictionsAndCalculateAccuracy(vs &givenData, vs &predictions)
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
 * Returns a vvs which contains information about
 * the data table. The vvs contains the names of
 * all the columns and the values that each
 * column can take
 */
vvs generateTableInfo(vvs &dataTable)
{   GENERATE_TABLE_INFO_ST = rdtsc();
	vvs tableInfo;
	for (int iii = 0; iii < dataTable[0].size(); iii++) {
		vs tempInfo;
		msi tempMap;
		for (int jjj = 0; jjj < dataTable.size(); jjj++) {
			if (tempMap.count(dataTable[jjj][iii]) == 0) {
				tempMap[dataTable[jjj][iii]] = 1;
				tempInfo.push_back(dataTable[jjj][iii]);
			} else	{
				tempMap[dataTable[jjj][iii]]++;
			}
		}
		tableInfo.push_back(tempInfo);
	}

	GENERATE_TABLE_INFO_ET = rdtsc() - GENERATE_TABLE_INFO_ST;
	GENERATE_TABLE_INFO += GENERATE_TABLE_INFO_ET;
	return tableInfo;
}

/*
 * Returns the most frequent class from the training data
 * This class will be used as the default class label
 */
string returnMostFrequentClass(vvs &dataTable)
{
	RETURN_MOST_FREQUENT_CLASS_ST = rdtsc();

	msi trainingClasses;           													 // Stores the classlabels and their frequency
	for (int iii = 1; iii < dataTable.size(); iii++) {
		if (trainingClasses.count(dataTable[iii][dataTable[0].size()-1]) == 0) {
			trainingClasses[dataTable[iii][dataTable[0].size()-1]] = 1;
		} else {
			trainingClasses[dataTable[iii][dataTable[0].size()-1]]++;
		}
	}   
	msi::iterator mapIter;
	int highestClassCount = 0;
	string mostFrequentClass;
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
	cout << "Return Column Index = " << RETURN_COLUMN_INDEX / RETURN_COLUMN_INDEX_COUNT << endl; 
	cout << "Return Index of Vector = " << RETURN_INDEX_OF_VECTOR / RETURN_INDEX_OF_VECTOR_COUNT << endl;
	cout << "Generate Table Info = " << GENERATE_TABLE_INFO << endl;
	cout << "Return Most Frequent Class = " << RETURN_MOST_FREQUENT_CLASS / RETURN_MOST_FREQUENT_CLASS_COUNT << endl;
}