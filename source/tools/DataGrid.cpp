/**
 * @file DataGrid.cpp
 * @brief Implementation of the DataGrid class.
 * @author Group 9
 */

#include "DataGrid.hpp"
#include <cassert>
#include <stdexcept>

namespace cse498 {

    /**
     * Allocates a grid of size r x c, all cells initialized to default Datum (NaN).
     * @param r number of rows
     * @param c number of columns
     */
    DataGrid::DataGrid(int r, int c) {
        mData = std::vector<std::vector<Datum>>(r, std::vector<Datum>(c));
        mEnd  = {0, 0};
        mDim  = {static_cast<size_t>(r), static_cast<size_t>(c)};
    }

    /**
     * Append a full row to the bottom of the grid.
     * If the grid has no columns yet (0x0), the row establishes the column width.
     * Otherwise the row must match NumCols(); a mismatch throws std::out_of_range.
     */
    void DataGrid::Append(const std::vector<Datum>& row) {
        if (mDim.second == 0 && mDim.first == 0) {
            mDim.second = row.size();

        } else if (row.size() != mDim.second) {
            throw std::out_of_range(std::format(
                "Append row width ({}) does not match grid column count ({})",
                row.size(), mDim.second));
        }
        
        mData.push_back(row);
        mDim.first++;
        mEnd = {mDim.first, 0};
    }

    /**
     * Get a copy of row r.
     */
    std::vector<Datum> DataGrid::Row(int r) const {
        assert(r >= 0 && static_cast<size_t>(r) < mDim.first);
        return mData[static_cast<size_t>(r)];
    }

    /**
     * Get a copy of column c, collected from each row.
     */
    std::vector<Datum> DataGrid::Column(int c) const {
        assert(c >= 0 && static_cast<size_t>(c) < mDim.second);
        std::vector<Datum> col;
        col.reserve(mDim.first);
        for (size_t i = 0; i < mDim.first; ++i) {
            col.push_back(mData[i][static_cast<size_t>(c)]);
        }
        return col;
    }

    /**
     * Return the Datum at (r, c). No bounds checking beyond what the vector provides.
     */
    Datum DataGrid::At(int r, int c) const {
        return mData[static_cast<size_t>(r)][static_cast<size_t>(c)];
    }

    size_t DataGrid::NumRows() const { return mDim.first; }
    size_t DataGrid::NumCols() const { return mDim.second; }

    DataGrid::Iterator      DataGrid::begin()       { return Iterator(this, 0, 0); }
    DataGrid::Iterator      DataGrid::end()         { return Iterator(this, mDim.first, 0); }
    DataGrid::ConstIterator DataGrid::begin()  const { return ConstIterator(this, 0, 0); }
    DataGrid::ConstIterator DataGrid::end()    const { return ConstIterator(this, mDim.first, 0); }
    DataGrid::ConstIterator DataGrid::cbegin() const { return begin(); }
    DataGrid::ConstIterator DataGrid::cend()   const { return end(); }

    // -------------------------------------------------------------------------
    // Iterator
    // -------------------------------------------------------------------------

    DataGrid::Iterator::Iterator(DataGrid* dg, size_t r, size_t c)
        : mDg(dg), mRow(r), mCol(c) {}

    bool DataGrid::Iterator::operator==(const Iterator& it) const {
        return mRow == it.mRow && mCol == it.mCol;
    }
    bool DataGrid::Iterator::operator!=(const Iterator& it) const {
        return !(*this == it);
    }
    Datum& DataGrid::Iterator::operator*() const {
        return mDg->mData[mRow][mCol];
    }
    DataGrid::Iterator& DataGrid::Iterator::operator++() {
        if (mDg->mDim.second == 0) return *this;
        if (mCol + 1 >= mDg->mDim.second) {
            mCol = 0;
            mRow++;
        } else {
            ++mCol;
        }
        return *this;
    }
    std::pair<size_t, size_t> DataGrid::Iterator::Pos() const {
        return {mRow, mCol};
    }

    // -------------------------------------------------------------------------
    // ConstIterator
    // -------------------------------------------------------------------------

    DataGrid::ConstIterator::ConstIterator(const DataGrid* dg, size_t r, size_t c)
        : mDg(dg), mRow(r), mCol(c) {}

    bool DataGrid::ConstIterator::operator==(const ConstIterator& it) const {
        return mRow == it.mRow && mCol == it.mCol;
    }
    bool DataGrid::ConstIterator::operator!=(const ConstIterator& it) const {
        return !(*this == it);
    }
    const Datum& DataGrid::ConstIterator::operator*() const {
        return mDg->mData[mRow][mCol];
    }
    DataGrid::ConstIterator& DataGrid::ConstIterator::operator++() {
        if (mDg->mDim.second == 0) return *this;
        if (mCol + 1 >= mDg->mDim.second) {
            mCol = 0;
            mRow++;
        } else {
            ++mCol;
        }
        return *this;
    }
    std::pair<size_t, size_t> DataGrid::ConstIterator::Pos() const {
        return {mRow, mCol};
    }

} // namespace cse498