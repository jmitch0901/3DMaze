#include "Maze.h"
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;

Maze::Maze(const string FILENAME){
	parseWasGood=false;
	ifstream inputStream(FILENAME);

	if(inputStream.good()){
		string line;
		int iteration = 0;

		numRows=-1;
		numColumns=-1;


		while(getline(inputStream,line)){
			iteration++;
			stringstream ss(line);
			
			if(iteration==1){ //For row count
				ss>>numRows;
				
			} else if (iteration==2){ //For column count
				ss>>numColumns;

			} else if (iteration==3){ //For starting and ending cells
				ss>>startingCell.rowNumber;
				ss>>startingCell.columnNumber;
				ss>>endingCell.rowNumber;
				ss>>endingCell.columnNumber;
			} else {
				int temp;
				while(ss>>temp){
					mazeCells.push_back(temp);
				}	
			}
		}

		inputStream.close();
		parseWasGood=true;


	} 
else {
		cout<<"Couldn't read the input stream: "+FILENAME<<endl;
	}

}

Maze::~Maze(){
	cout<<"Deconstructing Maze Object!"<<endl;
}

int Maze::getCellLogicAsInteger(const int columnNumber, const int rowNumber) const{
	int index = rowNumber*numColumns+columnNumber;
	return mazeCells[index];

}


void Maze::setCellLogicAsInteger(const int columnNumber, const int rowNumber, const int INT_CODE){
	int index = rowNumber*numColumns+columnNumber;
	mazeCells[index] = INT_CODE;
}


bool Maze::successfullyConstructed() const{
	return parseWasGood;
}

int Maze::getCellCount() const{
	return mazeCells.size();
}

int Maze::getColumnCount() const{
	return numColumns;
}

int Maze::getRowCount() const{
	return numRows;
}

