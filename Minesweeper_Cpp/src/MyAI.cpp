// ======================================================================
// FILE:        MyAI.cpp
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

#include "MyAI.hpp"

MyAI::MyAI ( int _rowDimension, int _colDimension, int _totalMines, int _agentX, int _agentY ) : Agent()
{
    // ======================================================================
    // YOUR CODE BEGINS
    // ======================================================================
    // Map size of current world
    rowDimension = _rowDimension;
    colDimension = _colDimension;
    bombLeft = _totalMines;
    tileCovered = rowDimension * colDimension - 2;

    // Map container for current world
    for (int r = 0; r < colDimension; r++) {
        vector<Tile> colTile;
        for (int c = 0; c < rowDimension; c++) {
            Tile t(r, c);
            colTile.push_back(t);
        }
        world.push_back(colTile);
    }

    // Record the beginning status
    currentTilePos.insert(currentTilePos.end(), {_agentX, _agentY});
    currentAction = UNCOVER;

    // ======================================================================
    // YOUR CODE ENDS
    // ======================================================================
};

Agent::Action MyAI::getAction( int number )
{
    // ======================================================================
    // YOUR CODE BEGINS
    // ======================================================================
    // Update current position mine number
    // cout << "go to: [" << currentTilePos[0]+1 << ", " << currentTilePos[1]+1 << "]" << endl;
    // cout << "tile covered: " << tileCovered << ", bomb left: " << bombLeft << endl;
    if (tileCovered == 1) {
        if (tileCovered == bombLeft)
        {
            if (!task.empty())
            {
                Agent::Action target = task.front();
                // cout << "[" << target.x << ", " << target.y << "]: " << target.action << endl;
                task.pop();
                return {FLAG, target.x, target.y};
            }
        }
    }

    if (currentAction == UNCOVER && !world[currentTilePos[0]][currentTilePos[1]].checkUncovered())
    {
        tileCovered--;
        vector<int> n{currentTilePos[0], currentTilePos[1]};
        world[currentTilePos[0]][currentTilePos[1]].uncover();
        world[currentTilePos[0]][currentTilePos[1]].updateMineCount(number);
        // cout << "[" << currentTilePos[0]+1 << ", " << currentTilePos[1]+1 << "]: UNCOVER" << endl;
    }
    if (currentAction == FLAG && !world[currentTilePos[0]][currentTilePos[1]].checkFlagged() && bombLeft > 0)
    {
        bombLeft--;
        world[currentTilePos[0]][currentTilePos[1]].flag();
        // cout << "[" << currentTilePos[0]+1 << ", " << currentTilePos[1]+1 << "]: FLAG" << endl;
    }

    // separate tiles by whether if they are surrounded by bombs.
    if(number == 0) {
        Tile node = Tile(currentTilePos[0], currentTilePos[1]);
        zeroBombNode.push(node);
    }
    else if (number > 0){ 
        Tile node = Tile(currentTilePos[0], currentTilePos[1]);
        bombDisposalNode.insert(node);
    }

    // Always processing tiles with no bomb surrounding them.
    if(zeroTask.empty()) {
        while(!zeroBombNode.empty()) {
            int currentX = zeroBombNode.front().getPosition()[0];
            int currentY = zeroBombNode.front().getPosition()[1];
            zeroBombNode.pop();
            scanNode(currentX, currentY, 0);
        }
    }

    // Find tiles: their number of bombs == number of covered tiles surrounding them
    if(task.empty()) {
        for(auto bdn : bombDisposalNode) {
            int currentX = bdn.getPosition()[0];
            int currentY = bdn.getPosition()[1];
            scanNode(currentX, currentY, 1);
        }
        bombDisposalNode.clear();
    }

    // Check again if tiles meet the conditions above
    if (task.empty())
    {
        while (!edgeNode.empty())
        {
            int currentX = edgeNode.front().getPosition()[0];
            int currentY = edgeNode.front().getPosition()[1];
            edgeNode.pop();
            scanNode(currentX, currentY, 2);
        }
    }

    // Calculate probabilities of bombs in each convered tile 
    // surrounding the tiles waiting for processing
    // (number of bombs < number of covered tiles)
    if (task.empty()) {
        for(auto bdn : bombDisposalNode){
            int currentX = bdn.getPosition()[0];
            int currentY = bdn.getPosition()[1];
            vector<int> p {currentX, currentY, 0};
            globalNodes.push_back(p);
        }
        bombDisposalNode.clear();

        // Isolate nodes into related graoup by relative distance
        vector<vector<pair<int, int>>> tileGroup;
        MyAI::groupTiles(tileGroup);
        vector<pair<int, vector<int>>> compareGroup;
        // Probability check to relative tiles
        if (globalNodes.size() > 0) {
            for (int i = 0; i < tileGroup.size(); i++) {
                if (tileGroup[i].size() > 30) continue;
                configurations.clear();
                // currentConfig.clear();
                vector<vector<int>> currentConfig;
                tankProbCheck(0, tileGroup[i], currentConfig);
                if (!configurations.empty()) {
                    int smallestX;
                    int smallestY;
                    int smallestP = 100000000;
                    for (int i = 0; i < configurations.size(); i++) {
                        if (smallestP > configurations[i][2]) {
                            smallestP = configurations[i][2];
                            smallestX = configurations[i][0];
                            smallestY = configurations[i][1];
                        }
                    }
                    vector<int> c {smallestX, smallestY};
                    pair<int, vector<int>> p1(smallestP, c);
                    compareGroup.push_back(p1);
                    // Agent::Action act;
                    // act = (Agent::Action){.action = UNCOVER, .x = smallestX, .y = smallestY};
                    // task.push(act);

                    // Add other nodes back to check again
                    for (int i = 0; i < globalNodes.size(); i++) {
                        // if(globalNodes[i][0] != smallestX && globalNodes[i][1] != smallestY){
                        Tile node = Tile(globalNodes[i][0], globalNodes[i][1]);
                        bombDisposalNode.insert(node);
                        // }
                    }
                }
            }
            if (!compareGroup.empty()) {
                vector<int> target = compareGroup[0].second;
                for (int i = 0; i < compareGroup.size() - 1; i++)
                {

                    if (compareGroup[i].first > compareGroup[i + 1].first)
                    {
                        target = compareGroup[i + 1].second;
                    }
                }
                Agent::Action act;
                act = (Agent::Action){.action = UNCOVER, .x = target[0], .y = target[1]};
                task.push(act);
            }
        }
    }
    globalNodes.clear();

    // always zero task first, then non-zero ones
    if (!zeroTask.empty()) {
        Agent::Action target = zeroTask.front();
        
        currentAction = target.action;
        currentTilePos.clear();
        currentTilePos.insert(currentTilePos.end(), {target.x, target.y});

        zeroTask.pop();
        return target;
    }
    if (!task.empty()) {
        Agent::Action target = task.front();
        
        // Update current position and action for the next move
        currentAction = target.action;
        currentTilePos.clear();
        currentTilePos.insert(currentTilePos.end(), {target.x, target.y});

        // Remove the task from our queue and do the move
        task.pop();
        return target;
    }

    // ======================================================================
    // IF ALL THE TASKS ARE DONE
    // ======================================================================
    return {LEAVE, -1, -1};

    // ======================================================================
    // YOUR CODE ENDS
    // ======================================================================

}

// ======================================================================
// YOUR CODE BEGINS
// ======================================================================
bool MyAI::distanceDetection(vector<int> node1, vector<int> node2){
    int dist = abs(node1[0] - node2[0]) + abs(node1[1] - node2[1]);
    if (dist > 3) {
        return false;
    }else if (dist == 3) {
        int x = abs(node1[0] - node2[0]);
        int y = abs(node1[1] - node2[1]);
        if (x == 3 || y == 3)
            return false;
    }
    return true;
}

void MyAI::groupTiles(vector<vector<pair<int, int>>>& tileGroup){
    if(globalNodes.empty()) return;

    // Union find
    vector<int> parent(globalNodes.size(), 0);
    for(int i = 0; i < parent.size(); i++){
        parent[i] = i;
    }
    for(int i = 0; i < parent.size() - 1; i++){
        for(int j = i + 1; j < parent.size(); j++){
            if(distanceDetection(globalNodes[i], globalNodes[j])){
                if(parent[i] < parent[j]){
                    parent[j] = parent[i];
                } else {
                    parent[i] = parent[j];
                }
            }
        }
    }

    map<int, vector<int>> group;
    for(int i = 0; i < parent.size(); i++){
        int r = i;
        while(parent[r] != r){
            r = parent[r];
        }
        if(group.find(r) == group.end()){
            group.insert({r, {i}});
        } else {
            group[r].push_back(i);
        }
    }

    int i = 0;
    for(auto g : group){
        tileGroup.push_back({});
        for(auto t : g.second){
            pair<int, int> e(globalNodes[t][0], globalNodes[t][1]);
            tileGroup[i].push_back(e);
        }
        i++;
    }
}

/* 
    0. if index == edgeNodes.size(), add current configuration to configurations
    1. Iterate through all edge nodes that are not obvious to check whether there's a bomb
    2. Check each edge node, scan its surrouding tiles, 
    2-1. if flag found, mine count minus 1
    2-2. else, covered tile count plus 1
    3. Check relationship between mine count and covered tile count
    3-1. if mine count > covered tile count, return (obvious it's not a reasonable pattern)
    3-2. else if mine count == covered tile count, 
    3-2-1. Mark all covered tiles as uncovered, 
    3-2-2. Recursively do tankProbcehck(index, edgeNodes, configurations)
    3-3. else if mine count < covered tile count, 
    3-3-1. Generate all permutations of flag(bomb) coordinates
    3-3-2. Mark as uncovered if corresponding position for coordinates in permutations is 0
           Mark as flagged if corresponding position for coordinates in permutations is 1
    3-3-2-1. Recursively do tankProbcehck(index, edgeNodes, configurations)
    4. 
 */
// void MyAI::tankProbCheck(int nodeIndex, vector<pair<int, int>>& candidateNodes, vector<vector<int>>& configurations, vector<vector<int>> currentConfig)
void MyAI::tankProbCheck(int nodeIndex, vector<pair<int, int>>& candidateNodes, vector<vector<int>> currentConfig) {
    
    if(nodeIndex == candidateNodes.size()) {
        for (int i = 0; i < currentConfig.size(); i++) {
            bool found = false;
            for (int j = 0; j < configurations.size(); j++) {
                if (configurations[j][0] == currentConfig[i][0] && configurations[j][1] == currentConfig[i][1]){
                    found = true;
                    configurations[j][2] += currentConfig[i][2];
                }
            }
            if(!found){
                vector<int> c {currentConfig[i][0], currentConfig[i][1], currentConfig[i][2]};
                configurations.push_back(c);
            }
        }
        return;
    }

    int currentX = candidateNodes[nodeIndex].first;
    int currentY = candidateNodes[nodeIndex].second;
    int mineCount = world[currentX][currentY].getMineCount();
    int coveredCount = 0;
    vector<vector<int>> coveredTiles;
    vector<int> combinations;

    // Calculate how many flags and covered tiles to the current position
    for(int i = 0; i < 3; i++) {
        int y_offset = i - 1;
        for(int j = 0; j < 3; j++)
        {
            int x_offset = j - 1;
            int x = currentX + x_offset;
            int y = currentY + y_offset;
            if (x >= 0 && x<colDimension && y>=0 && y<rowDimension && !(x==currentX && y==currentY)) {
                if (!world[x][y].checkUncovered() && !world[x][y].checkFlagged()) {
                    coveredCount++;
                    vector<int> pos{x, y};
                    coveredTiles.push_back(pos);
                }
                if (world[x][y].checkFlagged()) {
                    mineCount--;
                }
            }
        }
    }
    int nextNode = nodeIndex + 1;
    if(mineCount > coveredCount) {
        return;
    }
    else if(mineCount == coveredCount)
    {
        for(int i = 0; i < coveredTiles.size(); i++)
        {
            world[coveredTiles[i][0]][coveredTiles[i][1]].flag();
            vector<int> config {coveredTiles[i][0], coveredTiles[i][1], 1};
            currentConfig.push_back(config);
        }
        tankProbCheck(nextNode, candidateNodes, currentConfig);
        for (int i = 0; i < coveredTiles.size(); i++)
        {
            world[coveredTiles[i][0]][coveredTiles[i][1]].flag(false);
            currentConfig.pop_back();
        }
    }
    else { // mine count < covered tile count

        // Generate all combinations
        for (int i = 0; i < coveredCount; i++) {
            if (i < coveredCount - mineCount){
                combinations.push_back(0);
            }
            else {
                combinations.push_back(1);
            }
        }

        // Try all combinations
        vector<vector<int>> combinationGroup;
        do {
            vector<int> h;
            for(int i = 0; i < combinations.size(); i++) {
                h.push_back(combinations[i]);
            }
            combinationGroup.push_back(h);
        } while (next_permutation(combinations.begin(), combinations.end()));

        for(int i = 0; i < combinationGroup.size(); i++) {
            for (int j = 0; j < combinationGroup[i].size(); j++) {
                int flag = 0;
                if (combinationGroup[i][j] == 1) {
                    world[coveredTiles[j][0]][coveredTiles[j][1]].flag();
                    flag = 1;
                }
                else if (combinationGroup[i][j] == 0) {
                    world[coveredTiles[j][0]][coveredTiles[j][1]].uncover();
                    flag = 0;
                }
                vector<int> config{coveredTiles[j][0], coveredTiles[j][1], flag};
                currentConfig.push_back(config);
            }
            tankProbCheck(nextNode, candidateNodes, currentConfig);
            for (int j = 0; j < combinationGroup[i].size(); j++) {
                if (combinationGroup[i][j] == 1) {
                    world[coveredTiles[j][0]][coveredTiles[j][1]].flag(false);
                }
                else if (combinationGroup[i][j] == 0) {
                    world[coveredTiles[j][0]][coveredTiles[j][1]].uncover(false);
                }
                currentConfig.pop_back();
            }
        }
    }
}

void MyAI::scanNode(int currentX, int currentY, int level)
{
    int mineCount = world[currentX][currentY].getMineCount();
    int coveredCount = 0;
    int x_offset, y_offset, x, y;
    vector<vector<int>> nodes;

    for (int i = 0; i < 3; i++) {
        y_offset = i - 1;
        for (int j = 0; j < 3; j++) {
            x_offset = j - 1;
            x = currentX + x_offset;
            y = currentY + y_offset;

            if (x >= 0 && x < colDimension && y >= 0 && y < rowDimension && !(x == currentX && y == currentY)) {
                if(level == 0) { // tiles with zero bomb
                    vector<int> n {x, y};
                    if (!world[x][y].checkUncovered() && seenNodes.find(n) == seenNodes.end()) {
                        seenNodes.insert(n);
                        Agent::Action act;
                        act = (Agent::Action){.action = UNCOVER, .x = x, .y = y};
                        zeroTask.push(act);
                    }
                }else{
                    if (world[x][y].checkFlagged()) {
                        mineCount--;
                        continue;
                    }
                    vector<int> n{x, y};
                    if (!world[x][y].checkUncovered() && seenNodes.find(n) == seenNodes.end()) {
                        coveredCount++;
                        nodes.push_back(n);
                    }
                }
            }
        }
    }

    if(level > 0) {
        if (coveredCount == mineCount && coveredCount > 0 && mineCount > 0) {
            for (int k = 0; k < nodes.size(); k++) {
                Agent::Action act;
                act = (Agent::Action){.action = FLAG, .x = nodes[k][0], .y = nodes[k][1]};
                world[nodes[k][0]][nodes[k][1]].flag();
                task.push(act);

                vector<int> n{nodes[k][0], nodes[k][1]};
                seenNodes.insert(n);
            }
        }
        else if (coveredCount > 0 && mineCount == 0) {
            for (int k = 0; k < nodes.size(); k++) {
                Agent::Action act;
                act = (Agent::Action){.action = UNCOVER, .x = nodes[k][0], .y = nodes[k][1]};
                task.push(act);

                vector<int> n{nodes[k][0], nodes[k][1]};
                seenNodes.insert(n);
            }
        }
        else if (coveredCount > 0) {
            Tile tile = Tile(currentX, currentY);
            if (level == 1) {
                edgeNode.push(tile);
            }
            else if (level == 2) {
                bombDisposalNode.insert(tile);
            }
        }
    }
}

Tile::Tile(int pos_x, int pos_y)
{
    position_x = pos_x;
    position_y = pos_y;
    mine_count = -1;
    flagged = false;
    uncovered = false;
}

Tile::~Tile(){}

vector<int> Tile::getPosition()
{
    return vector<int> {position_x, position_y};
}

int Tile::getMineCount()
{
    return mine_count;
}
bool Tile::updateMineCount(int m_count)
{
    mine_count = m_count;
    return true;
}

void Tile::uncover(bool u)
{
    uncovered = u;
}
bool Tile::checkUncovered()
{
    return uncovered;
}

void Tile::flag(bool f)
{
    flagged = f;
}
bool Tile::checkFlagged()
{
    return flagged;
}

bool Tile::operator<(const Tile& t) const {
    if(this->position_x < t.position_x){
        return true;
    }
    else if (this->position_x == t.position_x){
        if (this->position_y < t.position_y){
            return true;
        }
    }
    return false;
}
// ======================================================================
// YOUR CODE ENDS
// ======================================================================
