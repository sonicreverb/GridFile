#include "GridFile.h"

unsigned int ExternalBlock::getElemsNum() {
    unsigned int result = 0;
    for (GridCell2D* cell : this->getCells())
        result += cell->getPoints().size();

    return result;
}

void ExternalBlock::insert(Point2D _point)
{
    for (GridCell2D* cell : this->getCells()) {
        Rectangle cellCoords = cell->getCoords();
        if (_point.x >= cellCoords.xStart && _point.y >= cellCoords.yStart && _point.x <= cellCoords.xEnd && _point.y <= cellCoords.yEnd) {
            cell->insert(_point);
            return;
        }
    }
}

bool GridFile2D::findObject(const Point2D& _point) {
    int column = 0;
    int row = 0;

    for (auto xSeparator : this->linearAxisScales[0]) {
        if (_point.x < xSeparator) break;
        else column++;
    }

    for (auto ySeparator : this->linearAxisScales[1]) {
        if (_point.y < ySeparator) break;
        else row++;
    }

    ExternalBlock* currentUnit = this->grid[row][column].second;
    for (GridCell2D* cell : currentUnit->getCells())
        for (const Point2D& point : cell->getPoints())
            if (_point.x == point.x && _point.y == _point.y) return true;

    return false;
}

vector<Point2D> GridFile2D::regionSearch(const Rectangle& _rect)
{
    vector<Point2D> result;

    int colStart = 0;
    int colEnd = this->linearAxisScales[0].size();

    for (int xID = 0; xID <= this->linearAxisScales[0].size(); xID++) {
        if (_rect.xStart > this->linearAxisScales[0][xID]) colStart = xID + 1;
        if (_rect.xStart < this->linearAxisScales[0][xID]) colEnd = xID;
    }

    int rowStart = 0;
    int rowEnd = this->linearAxisScales[1].size();

    for (int yID = 0; yID <= this->linearAxisScales[1].size(); yID++) {
        if (_rect.yStart > this->linearAxisScales[1][yID]) rowStart = yID + 1;
        if (_rect.yStart < this->linearAxisScales[1][yID]) rowEnd = yID;
    }

    vector<ExternalBlock*> blocks;
    for (int col = colStart; col <= colEnd; col++)
        for (int row = rowStart; row <= rowEnd; row++) {
            ExternalBlock* currentBlock = this->grid[row][col].second;
            bool alreadyAdded = false;

            for (ExternalBlock* addedBlock : blocks)
                if (currentBlock->getID() == addedBlock->getID()) {
                    alreadyAdded = true;
                    break;
                }

            if (!alreadyAdded) blocks.push_back(currentBlock);
        }

    for (ExternalBlock* block : blocks) {
        for (GridCell2D* cell : block->getCells())
            for (const Point2D& point : cell->getPoints())
                if (point.x >= _rect.xStart && point.y >= _rect.yStart && point.x <= _rect.xEnd && point.y <= _rect.yEnd)
                    result.push_back(point);
    }

    return result;
}

void GridFile2D::insert(Point2D _point)
{
    int column = 0;
    int row = 0;

    for (auto xSeparator : this->linearAxisScales[0]) {
        if (_point.x < xSeparator) break;
        else column++;
    }

    for (auto ySeparator : this->linearAxisScales[1]) {
        if (_point.y < ySeparator) break;
        else row++;
    }

    ExternalBlock* currentBlock = this->grid[row][column].second;
    for (GridCell2D* cell : currentBlock->getCells())
    {
        for (const Point2D& point : cell->getPoints())
            if (_point.x == point.x && _point.y == point.y) return;
    }
       

    //currentBlock->insert(_point);
    this->grid[row][column].first->insert(_point);
    if (currentBlock->isOverflowed())
        splitBlock(column, row);
}

void GridFile2D::splitCell(int _column, int _row)
{
    int n;
    double splitKey;

    n = (this->linearAxisScales[0].size() <= this->linearAxisScales[1].size()) ? 0 : 1;

    if (n == 0) {
        double xStart = this->grid[_row][_column].first->getCoords().xStart;
        double xEnd = this->grid[_row][_column].first->getCoords().xEnd;
        splitKey = (xStart + xEnd) / 2;
    }
    else {
        double yStart = this->grid[_row][_column].first->getCoords().yStart;
        double yEnd = this->grid[_row][_column].first->getCoords().yEnd;
        splitKey = (yStart + yEnd) / 2;
    }

    ExternalBlock* currentBlock = this->grid[_row][_column].second;
    
    ExternalBlock* newBlock = new ExternalBlock();
    GridCell2D* newCell = new GridCell2D();
    newBlock->getCells().push_back(newCell);
    newCell->setParent(newBlock);

    vector<Point2D> pointsToMove;

    for (GridCell2D* cell : currentBlock->getCells()) {
        vector<Point2D>& points = cell->getPoints();
        for (auto it = points.begin(); it != points.end();) {
            if ((n == 0 && it->x >= splitKey) || (n == 1 && it->y >= splitKey)) {
                pointsToMove.push_back(*it);
                it = points.erase(it);
            }
            else ++it;
        }
        cell->reBuildRectangle();
    }

    for (const Point2D& point : pointsToMove)
        newCell->insert(point);

    if (n == 0) {
        //this->linearAxisScales[0].resize(this->linearAxisScales[0].size() + 1);
        this->linearAxisScales[0].insert(this->linearAxisScales[0].begin() + _column, splitKey);

        for (size_t j = 0; j < this->grid.size(); ++j) {
            this->grid[j].insert(this->grid[j].begin() + _column + 1, this->grid[j][_column]);
        }

        this->grid[_row][_column + 1].first = newCell;
        this->grid[_row][_column + 1].second = newBlock;
    }
    else {
        //this->linearAxisScales[1].resize(this->linearAxisScales[1].size() + 1);
        this->linearAxisScales[1].insert(this->linearAxisScales[1].begin() + _row, splitKey);

        std::vector<std::pair<GridCell2D*, ExternalBlock*>> newRow(this->grid[0].size());
        for (size_t i = 0; i < this->grid[0].size(); ++i) {
            newRow[i] = this->grid[_row][i];
        }

        this->grid.insert(this->grid.begin() + _row + 1, newRow);
        this->grid[_row + 1][_column].first = newCell;
        this->grid[_row + 1][_column].second = newBlock;
    }
}

void GridFile2D::splitBlock(int _column, int _row)
{
    ExternalBlock* currentBlock = this->grid[_row][_column].second;
    while (currentBlock->isOverflowed()) {
        if (currentBlock->getCells().size() > 1) {
            int columnStart = this->grid.size();
            int columnEnd = -1;
            int rowStart = this->grid.size();
            int rowEnd = -1;

            for (int rowID = 0; rowID < this->grid.size(); rowID++) {
                for (int columnID = 0; columnID < this->grid.size(); columnID++) {
                    if (this->grid[rowID][columnID].second == currentBlock) {
                        if (rowID < rowStart) rowStart = rowID;
                        if (rowID > rowEnd) rowEnd = rowID;
                        if (columnID < columnStart) columnStart = columnID;
                        if (columnID > columnEnd) columnEnd = columnID;
                    }
                }
            }

            this->splitGroup(columnStart, rowStart, columnEnd, rowEnd);
        }
        else {
            this->splitCell(_column, _row);
        }
    }
}

void GridFile2D::splitGroup(int _columnStart, int _rowStart, int _columnEnd, int _rowEnd)
{
    ExternalBlock* currentBlock = this->grid[_rowStart][_columnStart].second;
    ExternalBlock* newBlock = new ExternalBlock();

    int columnSplit, rowSplit;

    if (_columnStart != _columnEnd) {
        columnSplit = _columnStart + (_columnEnd - _columnStart) / 2;
        rowSplit = _rowStart;
    }
    else {
        columnSplit = _columnStart;
        rowSplit = _rowStart + (_rowEnd - _rowStart) / 2;
    }

    for (int row = rowSplit; row <= _rowEnd; row++)
        for (int column = columnSplit; column <= _columnEnd; column++) {
            this->grid[row][column].second = newBlock;
            this->grid[row][column].first->setParent(newBlock);
            newBlock->getCells().push_back(this->grid[row][column].first);
        }
}

void GridFile2D::remove(Point2D _point)
{
    int n = this->linearAxisScales[0].size();
    int column = 0;
    while (column < n) {
        if (_point.x < this->linearAxisScales[0][column]) {
            break;
        }
        column++;
    }

    n = this->linearAxisScales[1].size();
    int row = 0;
    while (row < n) {
        if (_point.y < this->linearAxisScales[1][row]) {
            break;
        }
        row++;
    }
    ExternalBlock* currentBlock = this->grid[row][column].second;

    for (GridCell2D* cell : currentBlock->getCells()) {
        vector<Point2D> points = cell->getPoints();
        for (auto it = points.begin(); it != points.end();) {
            Point2D tmpPoint = *it;
            if (tmpPoint.x == _point.x && tmpPoint.y == _point.y) {
                it = points.erase(it);
                mergeBlock(column, row);
                return;
            }
            else {
                ++it;
            }
        }
    }
}

void GridFile2D::mergeBlock(int _column, int _row)
{
    ExternalBlock* currentBlock = this->grid[_row][_column].second;
    ExternalBlock* newBlock = findMergeCandidate(_column, _row);
    if (!newBlock) {
        return;
    }

    int n = currentBlock->getElemsNum();
    int nPrime = newBlock->getElemsNum();
    if ((n + nPrime) / static_cast<double>(ExternalBlock::capacity) > this->mergeThreshold) {
        return;
    }

    for (GridCell2D* cell : newBlock->getCells()) {
        for (const Point2D& point : cell->getPoints()) {
            currentBlock->insert(point);
        }
    }

    delete newBlock;
    replaceBlockReferences(newBlock, currentBlock);

    reduceGrid(_column, _row);
}

void GridFile2D::reduceGrid(int iCur, int jCur)
{
    bool mergeLeft = true, mergeRight = true;
    int n = this->linearAxisScales[1].size();

    for (int j = 0; j < n; ++j) {
        if (iCur == 0 || this->grid[j][iCur - 1] != this->grid[j][iCur]) {
            mergeLeft = false;
        }
        if (iCur == this->linearAxisScales[0].size() - 1 || this->grid[j][iCur + 1] != this->grid[j][iCur]) {
            mergeRight = false;
        }
    }

    if (mergeLeft) {
        this->linearAxisScales[0].erase(this->linearAxisScales[0].begin() + (iCur - 1));
        for (auto& row : this->grid) {
            row.erase(row.begin() + iCur);
        }
    }
    if (mergeRight) {
        this->linearAxisScales[0].erase(this->linearAxisScales[0].begin() + iCur);
        for (auto& row : this->grid) {
            row.erase(row.begin() + iCur);
        }
    }

    bool mergeUp = true, mergeDown = true;
    n = this->linearAxisScales[0].size();

    for (int i = 0; i < n; ++i) {
        if (jCur == 0 || this->grid[jCur - 1][i] != this->grid[jCur][i]) {
            mergeUp = false;
        }
        if (jCur == this->linearAxisScales[1].size() - 1 || this->grid[jCur + 1][i] != this->grid[jCur][i]) {
            mergeDown = false;
        }
    }

    if (mergeUp) {
        this->linearAxisScales[1].erase(this->linearAxisScales[1].begin() + (jCur - 1));
        this->grid.erase(this->grid.begin() + jCur);
    }
    if (mergeDown) {
        this->linearAxisScales[1].erase(this->linearAxisScales[1].begin() + jCur);
        this->grid.erase(this->grid.begin() + jCur);
    }
}

ExternalBlock* GridFile2D::findMergeCandidate(int _column, int _row)
{
    ExternalBlock* currentBlock = this->grid[_row][_column].second;
    ExternalBlock* candidateBlock = nullptr;

    if (_column > 0) {
        ExternalBlock* leftBlock = this->grid[_row][_column - 1].second;
        if (currentBlock != leftBlock && canMerge(currentBlock, leftBlock)) {
            return leftBlock;
        }
    }

    if (_column < this->linearAxisScales[0].size() - 1) {
        ExternalBlock* rightBlock = this->grid[_row][_column + 1].second;
        if (currentBlock != rightBlock && canMerge(currentBlock, rightBlock)) {
            return rightBlock;
        }
    }

    if (_row > 0) {
        ExternalBlock* topBlock = this->grid[_row - 1][_column].second;
        if (currentBlock != topBlock && canMerge(currentBlock, topBlock)) {
            return topBlock;
        }
    }

    if (_row < this->linearAxisScales[1].size() - 1) {
        ExternalBlock* bottomBlock = this->grid[_row + 1][_column].second;
        if (currentBlock != bottomBlock && canMerge(currentBlock, bottomBlock)) {
            return bottomBlock;
        }
    }

    return nullptr;
}

bool GridFile2D::canMerge(ExternalBlock* _block1, ExternalBlock* _block2)
{
    int totalElements = _block1->getElemsNum() + _block2->getElemsNum();
    return (totalElements / static_cast<double>(ExternalBlock::capacity)) <= this->mergeThreshold;
}

void GridFile2D::replaceBlockReferences(ExternalBlock* oldBlock, ExternalBlock* newBlock)
{
    for (auto& row : this->grid) {
        for (auto& cell : row) {
            if (cell.second == oldBlock) {
                cell.second = newBlock;
            }
        }
    }
}