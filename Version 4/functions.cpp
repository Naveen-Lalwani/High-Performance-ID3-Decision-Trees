#include "fast_code_utils.h"
#include "header.h"
#include <string.h>

static const int MAX_DEPTH = 8;
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

static __inline__ double LOG (double x, double y){
	double res;
   __asm__ __volatile__
     (	"fyl2x"
       : "=t" (res) : "0" (x), "u" (y) : "st(1)" );
     return res;
}



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

	int column = -1;
	vi headerRow;
	int rowSize = attributeTable[0].size();
	
	// Loop Fusion
	for (int i = 0; i < rowSize; ++i) {
		if (attributeTable[0][i] == colName) {
			column = i;
		} else {
			headerRow.push_back(attributeTable[0][i]);
		}
	}
	vvi prunedTable;
	prunedTable.push_back(headerRow);
	
	// Loop Peeling
	for (int i = 1; i < attributeTable.size(); ++i) {
		vi auxRow;
		if (attributeTable[i][column] == value) {
			for (int j = 0; j < rowSize; ++j) {
				if (j != column) {
					auxRow.push_back(attributeTable[i][j]);
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
		return NULL;
	}
	if (depth == MAX_DEPTH) {
		nodePtr->isLeaf = true;
		nodePtr->label = returnMostFrequentClass(table, encoded[featureCount - 1]);
		return nodePtr;
	}
	if (isHomogeneous(table)) {
		nodePtr->isLeaf = true;
		nodePtr->label = table[1][table[1].size() - 1];
		return nodePtr;
	} else {
		int splittingCol = decideSplittingColumn(table, encoded);
		nodePtr->splitOn = splittingCol;
		int colIndex = splittingCol - FEATURE_IDENTIFIER;
		
		for (int i = 0; i < encoded[colIndex]; ++i) {
			node* newNode = (node*) new node;
			newNode->label = i;
			nodePtr->childrenValues.push_back(i);
			newNode->isLeaf = false;
			newNode->splitOn = splittingCol;
			vvi auxTable = pruneTable(table, splittingCol, i);
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

	int iii = 1;
	int lastCol = table[0].size() - 1;
	int firstValue = table[1][lastCol];
	
	
	int first_vec[8] = {table[1][lastCol], table[1][lastCol], table[1][lastCol], table[1][lastCol], table[1][lastCol], table[1][lastCol], table[1][lastCol], table[1][lastCol]};
	__m256i first_int = _mm256_load_si256((__m256i*)(first_vec));
	__m256 first = _mm256_castsi256_ps(first_int);
	
	int bound = table.size()>>5;
	for (iii; iii < bound; iii+=32) {
		
		__m256i temp_vec1 = _mm256_load_si256((__m256i*)(&table[iii][lastCol]));
		__m256 temp1 = _mm256_castsi256_ps(temp_vec1);
		__m256i temp_vec2 = _mm256_load_si256((__m256i*)(&table[iii+8][lastCol]));
		__m256 temp2 = _mm256_castsi256_ps(temp_vec2);
		__m256i temp_vec3 = _mm256_load_si256((__m256i*)(&table[iii+16][lastCol]));
		__m256 temp3 = _mm256_castsi256_ps(temp_vec3);
		__m256i temp_vec4 = _mm256_load_si256((__m256i*)(&table[iii+24][lastCol]));
		__m256 temp4 = _mm256_castsi256_ps(temp_vec4);
		__m256 vcmp1 = _mm256_cmp_ps(first, temp1, 0);
		__m256 vcmp2 = _mm256_cmp_ps(first, temp2, 0);
		__m256 vcmp3 = _mm256_cmp_ps(first, temp3, 0);
		__m256 vcmp4 = _mm256_cmp_ps(first, temp4, 0);
		
		uint32_t vec_mask1 = _mm256_movemask_ps(vcmp1);
		uint32_t vec_mask2 = _mm256_movemask_ps(vcmp2);
		uint32_t vec_mask3 = _mm256_movemask_ps(vcmp3);
		uint32_t vec_mask4 = _mm256_movemask_ps(vcmp4);
		uint32_t mask = ~0;
		
		int popcnt = (mask ^ vec_mask1) + (mask ^ vec_mask2) + (mask ^ vec_mask3) + (mask ^ vec_mask4);
		if (popcnt != 0) {
			IS_HOMOGENEOUS_ET = rdtsc() - IS_HOMOGENEOUS_ST;
			IS_HOMOGENEOUS_COUNT++;
			IS_HOMOGENEOUS += IS_HOMOGENEOUS_ET;
			return false;
		}
	}
	
	while (iii < table.size()) {
		if (firstValue != table[iii][lastCol]) {
			IS_HOMOGENEOUS_ET = rdtsc() - IS_HOMOGENEOUS_ST;
			IS_HOMOGENEOUS_COUNT++;
			IS_HOMOGENEOUS += IS_HOMOGENEOUS_ET;
			return false;
		}
		iii  ++;
	}
	
	IS_HOMOGENEOUS_ET = rdtsc() - IS_HOMOGENEOUS_ST;
	IS_HOMOGENEOUS_COUNT++;
	IS_HOMOGENEOUS += IS_HOMOGENEOUS_ET;
	return true;
}

/** 
 * Check if a certain values exists in a function or not
 */
bool contains (vvi &table, int column, int value)
{

	int iii = 1;
	int lastCol = column;
	int firstValue = value;
	
	
	int first_vec[8] = {value, value, value, value, value, value, value, value};
	__m256i first_int = _mm256_load_si256((__m256i*)(first_vec));
	__m256 first = _mm256_castsi256_ps(first_int);
	
	int bound = table.size() >> 5;
	for (iii; iii < bound; iii += 32) {
		
		__m256i temp_vec1 = _mm256_load_si256((__m256i*)(&table[iii][lastCol]));
		__m256 temp1 = _mm256_castsi256_ps(temp_vec1);
		__m256i temp_vec2 = _mm256_load_si256((__m256i*)(&table[iii+8][lastCol]));
		__m256 temp2 = _mm256_castsi256_ps(temp_vec2);
		__m256i temp_vec3 = _mm256_load_si256((__m256i*)(&table[iii+16][lastCol]));
		__m256 temp3 = _mm256_castsi256_ps(temp_vec3);
		__m256i temp_vec4 = _mm256_load_si256((__m256i*)(&table[iii+24][lastCol]));
		__m256 temp4 = _mm256_castsi256_ps(temp_vec4);
		__m256 vcmp1 = _mm256_cmp_ps(first, temp1, 0);
		__m256 vcmp2 = _mm256_cmp_ps(first, temp2, 0);
		__m256 vcmp3 = _mm256_cmp_ps(first, temp3, 0);
		__m256 vcmp4 = _mm256_cmp_ps(first, temp4, 0);
		
		uint32_t vec_mask1 = _mm256_movemask_ps(vcmp1);
		uint32_t vec_mask2 = _mm256_movemask_ps(vcmp2);
		uint32_t vec_mask3 = _mm256_movemask_ps(vcmp3);
		uint32_t vec_mask4 = _mm256_movemask_ps(vcmp4);
		uint32_t mask = ~0;
		
		int popcnt = (mask ^ vec_mask1) + (mask ^ vec_mask2) + (mask ^ vec_mask3) + (mask ^ vec_mask4);
		if (popcnt == 0) {
			return true;
		}
	}
	
	while (iii < table.size()) {
		if (firstValue == table[iii][lastCol]) {
			return true;
		}
		iii++;
	}
	
	
	return false;
}

/*
 * Returns a vector of integers containing the counts
 * of all the various values of an attribute/column.
 */
vi countDistinct(vvi &table, int column, int totalValues) 
{
	COUNT_DISTINCT_ST = rdtsc();
	
	int unique[totalValues];
	memset(unique, 0, totalValues * sizeof(int));

	int bound = table.size() >> 4;
	int i;

	for(i = 1; i < bound; i += 16) {
       	++unique[table[i][column]];
       	++unique[table[i + 1][column]];
       	++unique[table[i + 2][column]];
       	++unique[table[i + 3][column]];
       	++unique[table[i + 4][column]];
       	++unique[table[i + 5][column]];
       	++unique[table[i + 6][column]];
       	++unique[table[i + 7][column]];
       	++unique[table[i + 8][column]];
       	++unique[table[i + 9][column]];
       	++unique[table[i + 10][column]];
       	++unique[table[i + 11][column]];
       	++unique[table[i + 12][column]];
       	++unique[table[i + 13][column]];
       	++unique[table[i + 14][column]];
       	++unique[table[i + 15][column]];
    }

	while (i < table.size()) {
		unique[table[i++][column]]++;
	}

	vi counts;
	for (i = 0; i < totalValues; ++i) {
		if (unique[i] != 0) {
			counts.push_back(unique[i]);
		}
	}
	counts.push_back(table.size() - 1);
 
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
int decideSplittingColumn(vvi &table, int encoded[])
{
	DECIDE_SPLITTING_COLUMN_ST = rdtsc();

	// Calculation of marginal Entropy
	double marginalEntropy = 0.0;
	
	vi output = countDistinct(table, table[0].size() - 1, encoded[featureCount - 1]);
	int outputLastVal = output.size() - 1;
	
	// Loop Unrolling
	int j;
	int count = ((outputLastVal & 0x1) == 0) ? outputLastVal -1: outputLastVal;
	
	for (j = 0; j < count; j = j + 2) {
		int temp1 = output[j];
		int temp2 = output[j + 1];
		marginalEntropy -= (LOG(temp1, temp1) - LOG(output[outputLastVal], temp1)) + (LOG(temp2, temp2) - LOG(output[outputLastVal], temp2));
	} 
	marginalEntropy = marginalEntropy / output[outputLastVal];
	
	double informationGain = -1;
	int splittingColumn = 0;

	for (int column = 0; column < table[0].size() - 1; ++column) {
		
		int colName = table[0][column];
		int totalValues = encoded[colName - FEATURE_IDENTIFIER];
		
		vi counts = countDistinct(table, column, totalValues);
		vd attributeEntropy;

		double conditionalEntropy = 0.0;
		for (int i = 0; i < totalValues; ++i) {
			double entropy = 0.0;
			
			if (contains(table, column, i)) { 				
				vvi tempTable = pruneTable(table, colName, i);
				vi classCounts = countDistinct(tempTable, tempTable[0].size() - 1, featureCount - 1);
				
				int classCountsLastVal = classCounts.size() - 1;
				
				count = ((classCountsLastVal & 0x1) == 0) ? classCountsLastVal - 1: classCountsLastVal;
				for (int k = 0; k < count; k = k + 2) {
					int temp1 = classCounts[k];
					int temp2 = classCounts[k + 1];
					entropy -= (LOG(temp1, temp1) - LOG(classCounts[classCountsLastVal], temp1)) + (LOG(temp2, temp2) - LOG(classCounts[classCountsLastVal], temp2));
				}
				
				attributeEntropy.push_back(entropy / classCounts[classCountsLastVal]);
				entropy = 0.0;
			}
		}
		for (int i = 0; i < counts.size() - 1; ++i) {
			conditionalEntropy += (counts[i] * attributeEntropy[i]);
		}
		conditionalEntropy = conditionalEntropy / counts[counts.size() - 1];
		
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

	for (int i = 0; i < stringVector.size(); ++i) {
		if (stringVector[i] == value)	{
			RETURN_INDEX_OF_VECTOR_ET = rdtsc() - RETURN_INDEX_OF_VECTOR_ST;
			RETURN_INDEX_OF_VECTOR += RETURN_INDEX_OF_VECTOR_ET;
			RETURN_INDEX_OF_VECTOR_COUNT++;

			return i;
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
	return (double) correct / givenData.size() * 100;
}


/*
 * Returns the most frequent class from the training data
 * This class will be used as the default class label
 */
int returnMostFrequentClass(vvi &table, int totalValues)
{
	RETURN_MOST_FREQUENT_CLASS_ST = rdtsc();

	int unique[totalValues];
	memset(unique, 0, totalValues * sizeof(int));

	int bound = table.size() >> 4;
	int i;
	int column = table[0].size() - 1;

	for(i = 1; i < bound; i += 16) {
       	++unique[table[i][column]];
       	++unique[table[i + 1][column]];
       	++unique[table[i + 2][column]];
       	++unique[table[i + 3][column]];
       	++unique[table[i + 4][column]];
       	++unique[table[i + 5][column]];
       	++unique[table[i + 6][column]];
       	++unique[table[i + 7][column]];
       	++unique[table[i + 8][column]];
       	++unique[table[i + 9][column]];
       	++unique[table[i + 10][column]];
       	++unique[table[i + 11][column]];
       	++unique[table[i + 12][column]];
       	++unique[table[i + 13][column]];
       	++unique[table[i + 14][column]];
       	++unique[table[i + 15][column]];
    }

	while (i < table.size()) {
		unique[table[i++][column]]++;
	}

	int highestClassCount = 0;
	int mostFrequentClass;
	for (i = 0; i < totalValues; ++i) {
		if (unique[i] >= highestClassCount) {
			highestClassCount = unique[i];
			mostFrequentClass = i;
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

