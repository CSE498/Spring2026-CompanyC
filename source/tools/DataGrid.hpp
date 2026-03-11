/**
* @file DataGrid.hpp
 * @brief Defines the DataGrid class, which manages a 2d grid of Datum objects.
 *
 * @author  Mitchell McAuley
 * @date    2/4/2026
 * @course  CSE 498
 */

#pragma once

#include<vector>
#include <memory>
#include <expected>
#include <format>
#include <algorithm>
#include <string>
#include <utility>
#include <ranges>

#include "Datum.hpp"


namespace cse498 {
    /**
     * Data structure representing a grid of almost anything.
     */
    class DataGrid {
    private:
        // Underlying data structure
        std::vector<std::vector<Datum>> mData;

        // The first unoccupied space in the grid
        std::pair<size_t, size_t> mEnd;
        // The dimensions of the grid
        std::pair<size_t, size_t> mDim;
    public:
        /**
         * Iterator implementation for the grid.
         */
        class Iterator {
        public:
            Iterator(DataGrid* dg, int r, int c) : mDg(dg), mRow(r), mCol(c) {}

            bool operator!=(const Iterator& it) const {
                return mRow != it.mRow || mCol != it.mCol;
            }

            bool operator==(const Iterator& it) const {
                return mRow == it.mRow && mCol == it.mCol;
            }

            Datum operator*() const {
                return mDg->mData[mRow][mCol];
            }

            Iterator& operator++()
            {
                if (mCol >= mDg->mDim.second - 1)
                {
                    mCol = 0;
                    mRow++;
                }else
                {
                    ++mCol;
                }
                return *this;
            }

            std::pair<int, int> Pos() { return std::make_pair(mRow, mCol); }

        private:
            DataGrid* mDg;
            int mRow;
            int mCol;
        };

        DataGrid(int r, int c);
        template <typename T> void Insert(T element);
        template <typename T> void Insert(int r, int col, T element);
        void Append(const std::vector<Datum> & row);

        std::vector<Datum> Row(int r) const;
        std::vector<Datum> Column(int c) const;

        [[nodiscard]] Datum At(int r, int c) const;
        template <typename T> std::expected<std::pair<int, int>, std::string> Find(T element);

        Iterator begin() {return Iterator(this, 0, 0); }
        Iterator end() {return Iterator(this, mEnd.first, mEnd.second); }
    };

    /**
     * Insert a new element into the bottom of the grid
     * @tparam T template
     * @param element the new element
     */
    template<typename T> void DataGrid::Insert(T element) {
        if (mEnd.second == 0 && mEnd.first != 0 && mEnd.first >= mDim.first) {
            mData.push_back(std::vector<Datum>{Datum(element)});
            mEnd.first++;
            mEnd.second = 1;
            mDim.first++;

            return;
        }
        mData[mEnd.first][mEnd.second] = Datum(element);
        mEnd.second++;
        if (mEnd.second == mDim.second) {
            mEnd.second = 0;
            mEnd.first++;
        }
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
        if (r >= mDim.first || c >= mDim.second || r < 0 || c < 0) {
            throw std::out_of_range(std::format("Indices outside the current grid. Input ({}, {}) in grid of size ({}, {})",
                    r, c, mDim.first, mDim.second));
        }

        if (mEnd.first < r || (mEnd.second <= c && mEnd.first == r)) {
            if (c + 1 >= mDim.second)
            {
                mEnd.second = 0;
                mEnd.first = r + 1;
            } else
            {
                mEnd.first = r;
                mEnd.second = c + 1;
            }
        }
        mData[r][c] = Datum(element);
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
        auto it = std::find_if(begin(), end(),
            [&key](const Datum & other){ return key.AsDouble() == other.AsDouble(); });

        //auto it = std::ranges::find_if(*this, [&key](const Datum & other){ return key.AsDouble() == other.AsDouble(); });

        if (it == end())
        {
            return std::unexpected("Your element is not in the data grid.");
        }

        return it.Pos();
    }
}