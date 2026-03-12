#include "Database.hpp"

namespace cse498{

    bool Database::Exists(const std::string & key) const {
        return mStorage.count(key) > 0; 
    }

    void Database::Clear(){
        mStorage.clear();
        
    }
    size_t Database::Size() const {
        return mStorage.size();
    }

    bool Database::Delete(const std::string & key) {
        if (!Exists(key)) return false; 
        mStorage.erase(key);
        return true;
    }
    



     



} //End namespace cse498