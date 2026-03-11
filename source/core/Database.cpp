#include "Database.hpp"

namespace cse498{

    bool Database::Exists(const std::string & key) const {
        return false;   
    }

    void Database::Clear(){
        mStorage.clear();
        mSize = 0;
    }
    size_t Database::Size() const {
        return mSize;
    }

     



} //End namespace cse498