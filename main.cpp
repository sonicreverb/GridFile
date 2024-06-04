#include "GridFile.h"

unsigned int GridCell2D::capacity = 3;
unsigned int ExternalBlock::capacity = 3;
unsigned int ExternalBlock::maxID = 0;

int main() {
    // Создание экземпляра GridFile2D
    GridFile2D gridFile;

    // Вставка точек
    gridFile.insert(Point2D{ 1.0, 1.0 });
    gridFile.insert(Point2D{ 2.0, 2.0 });
    gridFile.insert(Point2D{ 3.0, 3.0 });
    gridFile.insert(Point2D{ 4.0, 4.0 });
    gridFile.insert(Point2D{ 5.0, 5.0 });
    gridFile.insert(Point2D{ 6.0, 6.0 });
    gridFile.insert(Point2D{ 7.0, 7.0 });
    gridFile.insert(Point2D{ 8.0, 8.0 });
    gridFile.insert(Point2D{ 9.0, 9.0 });

    // Поиск точки
    Point2D searchPoint = { 5.0, 5.0 };

    // Региональный поиск точек
    Rectangle searchRegion = { 2.0, 2.0, 6.0, 6.0 };
    vector<Point2D> pointsInRegion = gridFile.regionSearch(searchRegion);

    // Удаление точки
    Point2D removePoint = { 3.0, 3.0 };
    gridFile.remove(removePoint);
}