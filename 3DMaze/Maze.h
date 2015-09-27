#ifndef _MY_MAZE_
#define _MY_MAZE_
#include <string>
#include <vector>
using namespace std;



struct coords{
	int rowNumber;
	int columnNumber;
};


/*
	Just simply holds the parceable data from the text. Translates into rows/columns by single index.
*/
class Maze{
private:

	vector<int> mazeCells;

	bool parseWasGood;

	int numRows;
	int numColumns;
	
	coords startingCell;
	coords endingCell;

public:
	Maze(const string FILENAME);
	bool successfullyConstructed() const;
	int getCellCount() const;

	int getColumnCount() const;
	int getRowCount() const;

	int getCellLogicAsInteger(const int columnNumber, const int rowNumber) const;
	void setCellLogicAsInteger(const int columnNumber, const int rowNumber, const int INT_CODE);


	int mazeIndexOf(const int columnNumber, const int rowNumber) const;
	
	~Maze();
};



#endif