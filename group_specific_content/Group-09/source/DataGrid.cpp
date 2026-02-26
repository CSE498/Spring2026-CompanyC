//
// Created by Mitchell McAuley on 2/4/2026.
//

#include "DataGrid.hpp"

#include <cassert>


namespace cse498 {
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
    std::vector<Datum> DataGrid::Row(int r) const {
        return (*mData)[r];
    }

    /**
     * Get a column of the grid.
     * @param c the index of the column you are trying to extract
     * @return a column from the grid
     */
    std::vector<Datum> DataGrid::Column(int c) const {
        assert(c >= 0 && c < mDim.second);

        std::vector<Datum> col;
        for (int i = 0; i < mDim.first; i++) {
            col.push_back((*mData)[i][c]);
        }

        return col;
    }

    /**
     * request a certain element from the grid.
     */
    Datum DataGrid::At(int r, int c) const {
        return (*mData)[r][c];
    }
}