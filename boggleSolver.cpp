#include <iostream>
#include <cmath>
#include <iterator>
#include <vector>
#include <algorithm>
#include <fstream>
#include <list>
using namespace std;

//#define DEBUG_

typedef std::vector<std::string> StringArray;
typedef StringArray GameBoard;

struct Vertex{
    typedef std::pair<size_t, size_t> Position;
    typedef std::list<Position> NeighborList;
    typedef NeighborList::const_iterator ConstNeighborIterator;
    typedef NeighborList::iterator NeighborIterator;
    Position position;
    NeighborList neighbors;
    Vertex(){};
    Vertex(const Position& p): position(p){}
    Vertex(const size_t row, const size_t col): position(Position(row,col)){}
};
enum MatchResult{
  NO_MATCH,
  MATCH_AND_IS_FULL_WORD,
  MATCH_AND_IS_JUST_PREFIX
};

typedef  std::vector<std::vector<Vertex> > Neighbors;

void usage();
StringArray readDictionary(const std::string& filePath);
GameBoard readBoard(const std::string& filePath);
Neighbors generateNeighbors(const GameBoard& board);
MatchResult isPrefixMatch(const StringArray& dictionary, const std::string& prefix);
MatchResult isWord(const StringArray& dictionary, const std::string& word);
bool isBoggleWord(const StringArray& dictionary,const std::string& prefix);

StringArray generateBoggleSolution(const GameBoard& board,
                                   const StringArray& dictionary,
                                   const Neighbors& neighbors);
//operators on current vertex
void generateBoggleSolution(std::string& currentPrefix,
                            std::vector<Vertex::Position>& usedPositionList,
                            StringArray& solution,
                            const Vertex::Position& currentVertex,
                            const GameBoard& board, 
                            const StringArray& dictionary, 
                            const Neighbors& neighbors);
template<typename Iterator>
void print(Iterator begin, Iterator end, const std::string& delim = " ");
ostream& operator <<(ostream& stream, const Vertex::Position& p){
   return stream << "(" << p.first << "," << p.second << ")";
}

int main(int argc, char *argv[])
{
    if(argc < 3){
        usage();
    }
    else
    {
        GameBoard board = readBoard(argv[1]);
        StringArray dictionary = readDictionary(argv[2]);
        Neighbors neighborMap = generateNeighbors(board);
#ifdef DEBUG_
        time_t clkBegin = clock();
#endif
        StringArray result = generateBoggleSolution(board,dictionary,neighborMap);
        std::sort(result.begin(), result.end());
        StringArray::iterator newEnd = std::unique(result.begin(), result.end());
        StringArray::iterator begin = result.begin();
        while(begin != newEnd){
            cout << *begin << endl;
            ++begin;
        }
#ifdef DEBUG_
        cout << "Time took = " << float(clock() - clkBegin) / float(CLOCKS_PER_SEC) << "s" << endl;
        cout << "Board size = " << board.size() << "x" << board.size() << endl;
        cout << "Number of words generated = " << std::distance(result.begin(), newEnd) << endl;
#endif
    }
    return 0;
}

void usage(){
    cout << "Usage =  <gameboard.txt> <dictionary.txt> " << endl;
}

StringArray generateBoggleSolution(const GameBoard& board,
                                   const StringArray& dictionary,
                                   const Neighbors& neighbors)
{
    StringArray solutionList;
    for(int row = 0; row < board.size(); ++row){
        for(int col = 0; col < board[row].size(); ++col){
            std::vector<Vertex::Position> usedList(1,Vertex::Position(row,col));
            std::string prefix(1,board[row][col]);
            generateBoggleSolution(prefix,usedList,solutionList,Vertex::Position(row,col),board,dictionary,neighbors);
        }
    }
    return solutionList;
}
void generateBoggleSolution(std::string& currentPrefix,
                            std::vector<Vertex::Position>& usedPositionList,
                            StringArray& solution,
                            const Vertex::Position& currentPosition,
                            const GameBoard& board, 
                            const StringArray& dictionary, 
                            const Neighbors& neighborMap)
{
    const std::list<Vertex::Position>& neighbors = neighborMap[currentPosition.first][currentPosition.second].neighbors;
    
    for(std::list<Vertex::Position>::const_iterator itr = neighbors.begin(); itr != neighbors.end(); ++itr)
    {   
        MatchResult matchResult =  isPrefixMatch(dictionary, currentPrefix);
        
        switch (matchResult) 
        {
            case MATCH_AND_IS_JUST_PREFIX:{
                if(currentPrefix.size() >= 3 && isWord(dictionary,currentPrefix) == MATCH_AND_IS_FULL_WORD){
                    solution.push_back(currentPrefix);
                }
                //continue if only neighbor isn't in our marked list
                if(std::find(usedPositionList.begin(), usedPositionList.end(), *itr) == usedPositionList.end())
                {
                    //add new letter to prefix
                    currentPrefix.push_back(board[itr->first][itr->second]);
                    //add position to used list
                    usedPositionList.push_back(*itr);
                    
                    generateBoggleSolution(currentPrefix,
                                           usedPositionList,
                                           solution,
                                           *itr,
                                           board,
                                           dictionary,
                                           neighborMap);
                    //undo changes
                    currentPrefix.erase(currentPrefix.end() - 1);
                    usedPositionList.erase(usedPositionList.end() - 1);
                }
                break;
            }
            default:
                break;
        }
        
    }
}

MatchResult isPrefixMatch(const StringArray& dictionary, const std::string& prefix)
{
    int start = 0, end = int(dictionary.size());
    while( start <= end){
        int mid = start + (end - start) / 2;       
        int compResult = dictionary[mid].compare(0,prefix.size(),prefix);
        if(compResult == 0 ){ 
            return MATCH_AND_IS_JUST_PREFIX;
        }else if(compResult < 0){
            start = mid + 1;
        }else{
            end = mid - 1;
        }
    }
    return  NO_MATCH;
}
MatchResult isWord(const StringArray& dictionary, const std::string& word){
    int start = 0, end = int(dictionary.size());
    while( start <= end){
        int mid = start + (end - start) / 2;
        if(dictionary[mid] == word){ 
            return MATCH_AND_IS_FULL_WORD;
        }else if(dictionary[mid] < word){
            start = mid + 1;
        }else{
            end = mid - 1;
        }
    }
    return  NO_MATCH; 
}
//generate map for constant time neighbor lookup
//note there's probably better way to do this, but i'm lazy right now
Neighbors generateNeighbors(const GameBoard& board){
    //assumtion nxn board
    Neighbors neighbors( board.size(), std::vector<Vertex>(board.size()));
    
    for (int row = 0; row < board.size() - 1; ++row) {
        for(int col = 0; col < board[row].size() - 1; ++col){
            Vertex& currentVertex = neighbors[row][col];
            //current to right and reverse
            currentVertex.neighbors.push_back(Vertex::Position(row,col + 1));
            neighbors[row][col+1].neighbors.push_back(Vertex::Position(row,col));
            
            //current to bottom right and reverse
            currentVertex.neighbors.push_back(Vertex::Position(row+1,col+1));
            neighbors[row+1][col+1].neighbors.push_back(Vertex::Position(row,col));
            
            //current to bottom
            currentVertex.neighbors.push_back(Vertex::Position(row+1,col));
            neighbors[row+1][col].neighbors.push_back(Vertex::Position(row,col));
        }
    }

    for (int row = 0; row < board.size() - 1; ++row) {
        for(int col = 1; col < board[row].size(); ++col){
            Vertex& currentVertex = neighbors[row][col];
            //current to bottom left and reverse
            currentVertex.neighbors.push_back(Vertex::Position(row+1,col-1));
            neighbors[row+1][col-1].neighbors.push_back(Vertex::Position(row,col));
        }
    }
    
    //for the last column
    for(int row = 0; row < board.size() - 1; ++row){
        int col = (int)board[row].size() - 1;
        Vertex& currentVertex = neighbors[row][col];
        //current to bottom and reverse
        currentVertex.neighbors.push_back(Vertex::Position(row+1,col));
        neighbors[row+1][col].neighbors.push_back(Vertex::Position(row,col));
    }
    //for the last row
    for(int col = 0; col < board.size() - 1; ++col){
        int row = (int)board.size() - 1;
        Vertex& currentVertex = neighbors[row][col];
        //currentEnd to right and reverse
        currentVertex.neighbors.push_back(Vertex::Position(row,col + 1));
        neighbors[row][col+1].neighbors.push_back(Vertex::Position(row,col));

    }
    for(int i = 0; i < board.size(); ++i){
        for(int j = 0; j < board[i].size(); ++j){
            neighbors[i][j].position = Vertex::Position(i,j);
        }
    }
    
    return neighbors;
}
GameBoard readBoard(const std::string& filePath){
    StringArray board;
    ifstream fileIn(filePath.c_str());
    std::copy(std::istream_iterator<std::string>(fileIn),
              std::istream_iterator<std::string>(),
              std::back_inserter(board));
    
    return board;
    
}
StringArray readDictionary(const std::string& filePath){
    ifstream fileIn(filePath.c_str());
    StringArray result;
    std::copy(  std::istream_iterator<std::string>(fileIn),
                std::istream_iterator<std::string>(),
                std::back_inserter(result));
    return result;
}
template<typename Iterator>
void print(Iterator begin, Iterator end, const std::string& delim){
    while(begin != end){
        cout << *begin << delim;
        ++begin;
    }
}