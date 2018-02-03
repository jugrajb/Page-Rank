
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "engine.h"


#define BUFSIZE 2048
#define BUFFER 128

/* Struct defination */
typedef struct  web_cell {
	double value;
} web_cell;

/* Function prototypes */
web_cell**    parse_file(FILE * web_file, int dimension);
int           get_maze_dimension(FILE * web_file);


int main(void) {
	/* Variables */
	char buffer[BUFSIZE + 1];
	Engine *ep = NULL; // A pointer to a MATLAB engine object
	mxArray *S = NULL, *Q = NULL, *M = NULL, *rank = NULL, *dim = NULL; // mxArray is the fundamental type underlying MATLAB data
		
	// Following code was adapated from the mazesolver.c takehome framework
	int error = 0;
	FILE *web_file = NULL;
	web_cell ** web = NULL;
	int dimension = 0;
	
	// Here is where you enter the connectivity matrix you want to test
	error = fopen_s(&web_file, "web.txt" , "r");
	if (error) {
		fprintf(stderr, "Unable to open file: %s\n",  "web.txt");
		system("pause");
		return 1;
	}


	if (web_file) {
		dimension = get_file_dimension(web_file);
		web = parse_file(web_file, dimension);
	}

	//**********************************************************************

	// This copies the dimension variable to an array which will be passed to MATLAB 
	double time4[1][1] = { {dimension} };

	
	/* Starts a MATLAB process*/
	if (!(ep = engOpen(NULL))) {
		fprintf(stderr, "\nCan't start MATLAB engine\n");
		system("pause");
		return 1;
	}

	int row = 0;
	int column = 0;
	int row2 = 0;
	// Dynamically allocates a new array to which the web array will be copied
	double *time1 = (double *)malloc(dimension* dimension* sizeof(double));

	// This copy's the web array to another array that will be copied to memory before being passed to MATLAB
	while (row < dimension) {
		while (column < dimension) {
			double temp = web[row][column].value;
			*(time1 + row*dimension + column) = temp;
			column++;
		}
		row++;
		column = 0;
	}
	
	// Copying values to arrays
	S = mxCreateDoubleMatrix(dimension, dimension, mxREAL);
	dim = mxCreateDoubleMatrix(1, 1, mxREAL);

	memcpy((void*)mxGetPr(S), (void*)time1, (dimension*dimension) * sizeof(double));
	memcpy((void*)mxGetPr(dim), (void*)time4, 1 * sizeof(double));

	// Putting variables in
	if (engPutVariable(ep, "S", S)) {
		fprintf(stderr, "\nCannot write S array to MATLAB \n");
		system("pause");
		exit(1);
	}

	if (engPutVariable(ep, "dimension", dim)) {
		fprintf(stderr, "\nCannot write dimension to MATLAB \n");
		system("pause");
		exit(1);
	}
	
	// Calculations using MATLAB engine
	if (engEvalString(ep, "Q(1:dimension,1:dimension) = 1/dimension")) {
		fprintf(stderr, "\nError calculating S \n");
		system("pause");
		exit(1);
	}

	if (engEvalString(ep, "rank(1:1,1:dimension)=1")) {
		fprintf(stderr, "\nError calculating S \n");
		system("pause");
		exit(1);
	}
	if (engEvalString(ep, "S = bsxfun(@rdivide, S, sum(S,2));")) {
		fprintf(stderr, "\nError calculating S \n");
		system("pause");
		exit(1);
	}

	if (engEvalString(ep, "S(isnan(S))=1/dimension")) {
		fprintf(stderr, "\nError calculating S \n");
		system("pause");
		exit(1);
	}

	if (engEvalString(ep, "S = S*0.85")) {
		fprintf(stderr, "\nError calculating S \n");
		system("pause");
		exit(1);
	}

	if (engEvalString(ep, "Q = Q*0.15")) {
		fprintf(stderr, "\nError calculating Q \n");
		system("pause");
		exit(1);
	}

	if (engEvalString(ep, "M = S+Q")) {
		fprintf(stderr, "\nError calculating M \n");
		system("pause");
		exit(1);
	}

	// This loops the rank = rank*M operation inoder to reach the desired matrix where the multiplication stops changing the result
	int loop = 100;
	while(loop > 0) {
		if (engEvalString(ep, "rank = rank'*M")) {
			fprintf(stderr, "\nError calculating rank \n");
			system("pause");
			exit(1);
		}
		if (engEvalString(ep, "rank = rank'")) {
			fprintf(stderr, "\nError calculating rank \n");
			system("pause");
			exit(1);
		}
		loop--;
	}
	

	// This divides the elments in the matrix by the sum of all elements in the column
	if (engEvalString(ep, "rank = rank./sum(rank)")) {
		fprintf(stderr, "\nError calculating rank \n");
		system("pause");
		exit(1);
	}
	
	// This prints the result
	if ((rank = engGetVariable(ep, "rank")) == NULL) {
		fprintf(stderr, "\nFailed to retrieve rank\n");
		system("pause");
		exit(1);
	}
	else {
		size_t sizeOfResult = mxGetNumberOfElements(rank);
		size_t i = 0;
		int flag = 0;
		int nodecounter = 1;
			
		if (flag == 0) {
			printf("Node     Rank\n---      ----\n");
			flag = 1;
		}
		
		for (i = 0; i < sizeOfResult; ++i) {
			
			printf("% d        %0.4f\n", nodecounter, *(mxGetPr(rank) + i));
			nodecounter++;
		}
		
		printf("\n\n");
	}


	// This code was used to debug and make sure the in-between matrix calculations for M, S, Q were correct
	
	//Prints the first Matrix
	printf("The S matrix is:\n");
	if ((S = engGetVariable(ep, "S")) == NULL) {
		fprintf(stderr, "\nFailed to retrieve matrix 1\n");
		system("pause");
		exit(1);
	}
	else {
		size_t sizeOfResult = mxGetNumberOfElements(S);
		size_t i = 0;
		int count = 0;

		for (i = 0; i < sizeOfResult; ++i) {
			if (i % dimension == 0 && count != 0) {
				printf("\n");
			}
			printf("%.4f ", *(mxGetPr(S) + i));
			count++;
		}
		printf("\n\n");
	}


	//Prints the second matrix
	printf("The  Q matrix is:\n");
	if ((Q = engGetVariable(ep, "Q")) == NULL) {
		fprintf(stderr, "\nFailed to matrix 1\n");
		system("pause");
		exit(1);
	}
	else {
		size_t sizeOfResult = mxGetNumberOfElements(Q);
		size_t i = 0;
		int count = 0;

		for (i = 0; i < sizeOfResult; ++i) {
			if (i % dimension == 0 && count != 0) {
				printf("\n");
			}
			printf("%.4f ", *(mxGetPr(Q) + i));
			count++;
		}
		printf("\n\n");
	}

	//Prints the third matrix
	printf("The M matrix is:\n");
	if ((M = engGetVariable(ep, "M")) == NULL) {
		fprintf(stderr, "\nFailed to matrix M\n");
		system("pause");
		exit(1);
	}
	else {
		size_t sizeOfResult = mxGetNumberOfElements(M);
		size_t i = 0;
		int count = 0;

		for (i = 0; i < sizeOfResult; ++i) {
			if (i % dimension == 0 && count != 0) {
				printf("\n");
			}
			printf("%.4f ", *(mxGetPr(M) + i));
			count++;
		}
		printf("\n\n");
	}
	

	if (engOutputBuffer(ep, buffer, BUFSIZE)) {
		fprintf(stderr, "\nCan't create buffer for MATLAB output\n");
		system("pause");
		return 1;
	}
	buffer[BUFSIZE] = '\0';
	

	engEvalString(ep, "whos");
	printf("%s\n", buffer);

	//This clears the matrixs
	mxDestroyArray(M);
	mxDestroyArray(Q);
	mxDestroyArray(S);
	mxDestroyArray(rank);
	mxDestroyArray(dim);
	dim = NULL;
	M = NULL;
	Q = NULL;
	S = NULL;
	rank = NULL;

	if (engClose(ep)) {
		fprintf(stderr, "\nFailed to close MATLAB engine\n");
	}

	system("pause");
	return 0;
}


//This code was adapted from the mazesolver.c framework
/*
Acquires and returns the web.txt size.  Since the web is always a square, all we
need to do is find the length of the top row!
PARM:      web_file is a pointer to a filestream
PRE:       web_file is an initialized pointer to a correctly-formatted web.txt file
POST:      web_file's internal pointer is set to beginning of stream
RETURN:    length of the first line of text in the web.txt file EXCLUDING any EOL characters
('\n' or '\r') and EXCLUDING the string-terminating null ('\0') character.
*/
int get_file_dimension(FILE* web_file) {

	int  dimension = 0;
	char line_buffer[BUFFER];

	dimension = strlen(fgets(line_buffer, BUFFER, web_file));

	/* You don't need to know this.  It 'resets' the file's internal pointer to the
	beginning of the file. */
	fseek(web_file, 0, SEEK_SET);

	/* Checks if text file was created in Windows and contains '\r'
	IF TRUE reduce strlen by 2 in order to omit '\r' and '\n' from each line
	ELSE    reduce strlen by 1 in order to omit '\n' from each line */
	if (strchr(line_buffer, '\r') != NULL) {
		// INSERT CODE HERE (1 line)
		return strlen(line_buffer) - 2;
	}
	else {
		// INSERT CODE HERE (1 line)
		return strlen(line_buffer) - 1;
	}
}

// This code was adapted from the mazesolver.c framework
/*
Parses and stores web as a 2D array of web_cell.  This requires a few steps:
1) Allocating memory for a 2D array of web_cell, e.g., web_cell[rows][columns]
a) Dynamically allocates memory for 'dimension' pointers to web_cell, and assign
the memory (case as a double pointer to web_cell) to web, which is a
double pointer to web_cell (this makes the web[rows] headers)
b) For each row of the web, dynamically allocate memory for 'dimension' web_cells
and assign it (cast as a pointer to web_cell) to web[row]
2) Copying the file to the allocated space
a) For each row obtained from the file using fgets and temporarily stored in line_buffer
i) For each of the 'dimension' columns in that row
3) Returns the double pointer called web.
PARAM:  web_file pointer to an open filestream
PARAM:  dimension pointer to an int
PRE:    web_file is a pointer to a correctly-formatted web.txt file
POST:   dimension contains the correct size of the square connectivity matrix
POST:   web contains a dynamically allocated copy of the web stored in the web_file
RETURN: web, a web_cell double pointer, which points to 'dimension' single pointers
to web_cell, each of which points to 'dimension' web_cell structs.
*/

web_cell** parse_file(FILE * web_file, int dimension)
{
	/* Variables */
	char         line_buffer[BUFFER];
	int          row = 0,
		
	column = 0;
	web_cell ** web = NULL;

	/* Allocates memory for correctly-sized conenctivity matrix */
	web = (web_cell **)calloc(dimension, sizeof(web_cell*));


	for (row = 0; row < dimension; ++row) {
		web[row] = (web_cell*)calloc(dimension, sizeof(web_cell));
	}

	/* Copies connectivity matrix file to memory */
	row = 0;
	while (fgets(line_buffer, BUFFER, web_file)) {
		for (column = 0; column < dimension; ++column) {
			web[row][column].value = line_buffer[column]-48;
		}
		row++;
	}
	return web;
}