#include "constants.h"
#include "matrixPoint.h"

bool MatrixPoint::isCorrect() const {
    return row >= 0 && row < ROW_SIZE && col >= 0 && col < COL_SIZE;
}
