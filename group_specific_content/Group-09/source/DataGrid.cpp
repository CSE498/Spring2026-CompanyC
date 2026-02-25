//
// Created by mitch on 2/4/2026.
//

#include "DataGrid.hpp"

#include <format>
#include <algorithm>

/**
 * Allocates a grid of size r * c
 * @param r the vertical capacity
 * @param c the horizontal capacity
 */
DataGrid::DataGrid(int r, int c) {
    mData = std::make_unique<std::vector<std::vector<Datum>>>(r, std::vector<Datum>(c));
    mEnd = {0,0};
    mDim = {r, c};
}

/**
 * Insert a new element into the bottom of the grid
 * @tparam T template
 * @param element the new element
 */
template<typename T> void DataGrid::Insert(T element) {
    if (mEnd == mDim) {
        mData->push_back(std::vector<Datum>{Datum(element)});
        mEnd.first++;
        mEnd.second = 1;
        mDim.first++;
    }
    (*mData)[mEnd.first][mEnd.second] = Datum(element);
}

/**
 * Insert an element into the grid at a specific spot.
 * @tparam T template
 * @param r row index
 * @param c column index
 * @param element
 */
template<typename T>
void DataGrid::Insert(int r, int c, T element) {
    if (r >= mDim.first || c >= mDim.second) {
        throw std::out_of_range(std::format("Indices outside the current grid. Input ({}, {}) in grid of size ({}, {})",
                r, c, mDim.first, mDim.second));
    }

    if (mEnd.second < c || mEnd.first < r) {
        mEnd.first = r;
        mEnd.second = c;
    }
    (*mData)[r][c] = Datum(element);
}

/**
 *  add a whole row to the grid
 * @param row row
 */
void DataGrid::Append(const std::vector<Datum> & row) {
    mData->push_back(row);
    mDim.first++;
}

/**
 * Get a row from the grid.
 * @param r the index of the row you are trying to extract
 * @return a row from the grid
 */
std::vector<Datum> DataGrid::Row(int r) {
    return (*mData)[r];
}

/**
 * Get a column of the grid.
 * @param c the index of the column you are trying to extract
 * @return a column from the grid
 */
std::vector<Datum> DataGrid::Column(int c) {
    std::vector<Datum> col;
    for (int i = 0; i < mDim.first; i++) {
        col.push_back((*mData)[i][c]);
    }

    return col;
}

/**
 * request a certain element from the grid.
 */
Datum DataGrid::At(int r, int c) {
    return (*mData)[r][c];
}

/**
 *
 * @tparam T template type for input
 * @param element The value being searched for in the grid
 * @return either a pair of the form (row, column) or an error message.
 */
template<typename T>
std::expected<std::pair<int, int>, std::string> DataGrid::Find(T element) {
    Datum key(element);
    auto it = std::find_if(mData->begin(), mData->end(),
        [&key](const Datum & other){ return key.AsDouble() == other.AsDouble(); });

    if (it == mData->end())
    {
        return std::unexpected("Your element is not in the data grid.");
    }

    return it.Pos();
}
