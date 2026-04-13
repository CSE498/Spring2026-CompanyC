/**
 * @file DataGrid.hpp
 * @brief Defines the DataGrid class, which manages a 2d grid of Datum objects.
 * @author Group 9
 */

#pragma once

#include <vector>
#include <memory>
#include <expected>
#include <format>
#include <algorithm>
#include <string>
#include <utility>
#include <ranges>
#include <iterator>
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
         * Forward iterator over all Datum elements in row-major order.
         */
        class Iterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type        = Datum;
            using difference_type   = std::ptrdiff_t;
            using pointer           = Datum*;
            using reference         = Datum&;

            Iterator(DataGrid* dg, size_t r, size_t c);

            bool operator!=(const Iterator& it) const;
            bool operator==(const Iterator& it) const;
            Datum& operator*() const;
            Iterator& operator++();
            std::pair<size_t, size_t> Pos() const;

        private:
            DataGrid* mDg;
            size_t mRow;
            size_t mCol;
        };

        /**
         * Const forward iterator — needed for read-only grids (e.g. Load<DataGrid>).
         */
        class ConstIterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type        = Datum;
            using difference_type   = std::ptrdiff_t;
            using pointer           = const Datum*;
            using reference         = const Datum&;

            ConstIterator(const DataGrid* dg, size_t r, size_t c);

            bool operator!=(const ConstIterator& it) const;
            bool operator==(const ConstIterator& it) const;
            const Datum& operator*() const;
            ConstIterator& operator++();
            std::pair<size_t, size_t> Pos() const;

        private:
            const DataGrid* mDg;
            size_t mRow;
            size_t mCol;
        };

        DataGrid(int r, int c);
        template <typename T> void Insert(T element);
        template <typename T> void Insert(int r, int c, T element);
        void Append(const std::vector<Datum>& row);

        std::vector<Datum> Row(int r) const;
        std::vector<Datum> Column(int c) const;

        [[nodiscard]] Datum At(int r, int c) const;
        template <typename T> std::expected<std::pair<int, int>, std::string> Find(T element) const;

        [[nodiscard]] size_t NumRows() const;
        [[nodiscard]] size_t NumCols() const;

        Iterator      begin();
        Iterator      end();
        ConstIterator begin()  const;
        ConstIterator end()    const;
        ConstIterator cbegin() const;
        ConstIterator cend()   const;
    };

    // -------------------------------------------------------------------------
    // Template implementations
    // -------------------------------------------------------------------------

    /**
     * Insert a new element sequentially (fills row-major into pre-allocated grid).
     */
    template<typename T>
    void DataGrid::Insert(T element) {
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
     * Insert an element at a specific (row, col) position.
     * Throws std::out_of_range if indices are outside the grid.
     */
    template<typename T>
    void DataGrid::Insert(int r, int c, T element) {
        auto ur = static_cast<size_t>(r);
        auto uc = static_cast<size_t>(c);

        if (r < 0 || c < 0 || ur >= mDim.first || uc >= mDim.second) {
            throw std::out_of_range(std::format(
                "Indices outside the current grid. Input ({}, {}) in grid of size ({}, {})",
                ur, uc, mDim.first, mDim.second));
        }

        if (mEnd.first < ur || (mEnd.first == ur && mEnd.second <= uc)) {
            if (uc + 1 >= mDim.second) {
                mEnd.second = 0;
                mEnd.first = ur + 1;
            } else {
                mEnd.first = ur;
                mEnd.second = uc + 1;
            }
        }

        mData[ur][uc] = Datum(element);
    }

    /**
     * Search for a value in the grid.
     * Compares strings as strings, everything else as doubles.
     * Returns (row, col) of first match, or an error string if not found.
     */
    template<typename T>
    std::expected<std::pair<int, int>, std::string> DataGrid::Find(T element) const {
        Datum key(element);

        for (size_t r = 0; r < mDim.first; ++r) {
            for (size_t c = 0; c < mDim.second; ++c) {
                const Datum& cell = mData[r][c];
                bool match = false;

                if (key.IsString() && cell.IsString()) {
                    match = (key.AsString() == cell.AsString());
                } else {
                    double kd = key.AsDouble();
                    double cd = cell.AsDouble();
                    // avoid NaN == NaN being true
                    match = !std::isnan(kd) && !std::isnan(cd) && (kd == cd);
                }

                if (match) {
                    return std::pair<int, int>{static_cast<int>(r), static_cast<int>(c)};
                }
            }
        }

        return std::unexpected("Element not found in DataGrid.");
    }

} // namespace cse498