// ======================================================================
// FILE:        MyAI.hpp
//
// AUTHOR:      Jian Li
//
// DESCRIPTION: This file contains your agent class, which you will
//              implement. You are responsible for implementing the
//              'getAction' function and any helper methods you feel you
//              need.
//
// NOTES:       - If you are having trouble understanding how the shell
//                works, look at the other parts of the code, as well as
//                the documentation.
//
//              - You are only allowed to make changes to this portion of
//                the code. Any changes to other portions of the code will
//                be lost when the tournament runs your code.
// ======================================================================

#ifndef MINE_SWEEPER_CPP_SHELL_MYAI_HPP
#define MINE_SWEEPER_CPP_SHELL_MYAI_HPP

#include "Agent.hpp"
#include <iostream> // temporary use
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <queue>

using namespace std;

class Tile
{
private:
  int position_x;
  int position_y;
  int mine_count;
  bool uncovered;
  bool flagged;

public:
  Tile(int pos_x, int pos_y);
  ~Tile();
  vector<int> getPosition();
  int getMineCount();
  bool updateMineCount(int mineCount);
  void uncover(bool u=true);
  bool checkUncovered();
  void flag(bool f=true);
  bool checkFlagged();
  bool operator<(const Tile& t1) const;
};

class MyAI : public Agent
{
public:
    MyAI ( int _rowDimension, int _colDimension, int _totalMines, int _agentX, int _agentY );

    Action getAction ( int number ) override;


    // ======================================================================
    // YOUR CODE BEGINS
    // ======================================================================
    void scanAddNode(int x, int y);
    void scanNode(int x, int y, int l);
    void tankProbCheck(int index, vector<pair<int, int>> &candidate, vector<vector<int>> currentConfig);
    void groupTiles(vector<vector<pair<int, int>>>& tilesGroup);
    bool distanceDetection(vector<int> node1, vector<int> node2);

  private:
    map<vector<int>, int> probTable;
    vector<vector<Tile>> world;

    set<vector<int>> seenNodes;
    queue<Agent::Action> task;
    queue<Agent::Action> zeroTask;
    queue<Tile> zeroBombNode;
    set<Tile> bombDisposalNode;
    queue<Tile> edgeNode;
    vector<vector<int>> globalNodes;
    vector<vector<int>> configurations{};
    // vector<vector<int>> currentConfig{};
    int depthLimit = 0;
    int bombLeft;
    int tileCovered;
    
    vector<int> currentTilePos;
    Agent::Action_type currentAction;
    // ======================================================================
    // YOUR CODE ENDS
    // ======================================================================
};

#endif //MINE_SWEEPER_CPP_SHELL_MYAI_HPP
