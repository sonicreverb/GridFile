#pragma once
#include <vector>
#include <cfloat>
#include <cmath>

using namespace std;

struct Point2D {
    double x, y;
};

struct Rectangle {
    double xStart, yStart, xEnd, yEnd;
};

class ExternalBlock;

class GridCell2D {
private:
    double xStart, yStart, xEnd, yEnd;
    vector<Point2D> points;
    ExternalBlock* parent;
public:
    static unsigned int capacity;
    GridCell2D() : xStart(DBL_MIN), yStart(DBL_MIN), xEnd(DBL_MAX), yEnd(DBL_MAX), parent(nullptr) { points.reserve(capacity); }
    GridCell2D(double _xStart, double _yStart, double _xEnd, double _yEnd) : xStart(_xStart), yStart(_yStart), xEnd(_xEnd), yEnd(_yEnd), parent(nullptr) { points.reserve(capacity); }

    vector<Point2D>& getPoints() { return this->points; }
    Rectangle getCoords() const {
        Rectangle result = { xStart, yStart, xEnd, yEnd };
        return result;
    }
    void reBuildRectangle() {
        this->xStart = DBL_MIN;
        this->xEnd = DBL_MAX;
        this->yStart = DBL_MIN;
        this->yEnd = DBL_MAX;

        for (const Point2D& point : this->points) {
            if (point.x < xStart || xStart == DBL_MIN) xStart = point.x;
            if (point.x > xEnd || xEnd == DBL_MAX) xEnd = point.x;
            if (point.y < yStart || yStart == DBL_MIN) yStart = point.y;
            if (point.y > yEnd || yEnd == DBL_MAX) yEnd = point.y;
        }

    }

    void insert(Point2D _point) {
        this->points.push_back(_point);
        if (_point.x < xStart || xStart == DBL_MIN) xStart = _point.x;
        if (_point.x > xEnd || xEnd == DBL_MAX) xEnd = _point.x;
        if (_point.y < yStart || yStart == DBL_MIN) yStart = _point.y;
        if (_point.y > yEnd || yEnd == DBL_MAX) yEnd = _point.y;
    }
    void setParent(ExternalBlock* _parent) { this->parent = _parent; }
};

class ExternalBlock {
private:
    vector<GridCell2D*> cells;
    unsigned int ID;
    static unsigned int maxID;
public:
    static unsigned int capacity;
    ExternalBlock() {
        this->increaseID();
        this->ID = maxID;
    }

    ~ExternalBlock() {
        for (auto cell : cells) {
            delete cell;
        }
    }

    vector<GridCell2D*>& getCells() { return this->cells; }
    unsigned int getID() const { return this->ID; }
    unsigned int getElemsNum();
    bool isOverflowed() { return this->getElemsNum() > capacity; }

    static void increaseID() { maxID += 1; }

    void insert(Point2D _point);
};

class GridFile2D {
private:
    vector<vector<double>> linearAxisScales;
    vector<vector<pair<GridCell2D*, ExternalBlock*>>> grid;
    double mergeThreshold;
public:
    GridFile2D() : mergeThreshold(0.5) {
        ExternalBlock* initialBlock = new ExternalBlock();
        GridCell2D* initialCell = new GridCell2D();
        initialBlock->getCells().push_back(initialCell);
        initialCell->setParent(initialBlock);

        grid.resize(1);
        grid[0].push_back(make_pair(initialCell, initialBlock));

        linearAxisScales.resize(2);
    }

    ~GridFile2D() {
        for (auto& row : grid) {
            for (auto& cellPair : row) {
                delete cellPair.second; // ExternalBlock деструктор удалит GridCell2D
            }
        }
    }

    bool findObject(const Point2D& _point);
    vector<Point2D> regionSearch(const Rectangle& _rect);
    void insert(Point2D _point);

    void splitCell(int _column, int _row);
    void splitBlock(int _column, int _row);
    void splitGroup(int _columnStart, int _rowStart, int _columnEnd, int _rowEnd);

    void remove(Point2D _point);
    void mergeBlock(int _column, int _row);
    bool canMerge(ExternalBlock* _block1, ExternalBlock* _block2);
    ExternalBlock* findMergeCandidate(int _column, int _row);
    void reduceGrid(int _column, int _row);
    void replaceBlockReferences(ExternalBlock* oldBlock, ExternalBlock* newBlock);
};
