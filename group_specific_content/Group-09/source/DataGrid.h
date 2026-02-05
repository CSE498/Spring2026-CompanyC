//
// Created by mitch on 2/4/2026.
//

#ifndef UNTITLED_DATAGRID_H
#define UNTITLED_DATAGRID_H

#include<vector>
#include <memory>

#include "Datum.h"

class DataGrid {
private:
    // Underlying data structure
    std::unique_ptr<std::vector<std::vector<Datum>>> data;

    // The first unoccupied space in the grid
    std::pair<int, int> mEnd;
    // The dimensions of the grid
    std::pair<int, int> mDim;
public:
    /**
     * Iterator implentation for the grid.
     */
    class Iterator {
    public:
        Iterator(DataGrid* dg, int r, int c) : mDg(dg), mRow(r), mCol(c) {}

        bool operator!=(const Iterator& it) {
            return mRow != it.mRow || mCol != it.mCol;
        }

        Datum operator*() const {
            return (*mDg->data)[mRow][mCol];
        }

    private:
        DataGrid* mDg;
        int mRow;
        int mCol;
    };

    DataGrid(int r, int c);
    template <typename T> void Insert(T element);
    template <typename T> void Insert(int r, int col, T element);
    void Append(std::vector<Datum> row);

    std::vector<Datum> Row(int r);
    std::vector<Datum> Column(int c);

    template <typename T> T At(int r, int c);
    template <typename T> std::pair<int, int> Find(T element);

    Iterator begin() {return Iterator(this, 0, 0); }
    Iterator end() {return Iterator(this, mEnd.first, mEnd.second); }
};


#endif //UNTITLED_DATAGRID_H