#include "DataMap.hpp"


void cse498::DataMap::SetData(const std::string& key, const std::any& value) {
    mDataMap[key] = value;
}


bool cse498::DataMap::Contains(const std::string& key) const {
    return mDataMap.find(key) != mDataMap.end();
}


void cse498::DataMap::RemoveData(const std::string& key) {
    mDataMap.erase(key);
}


void cse498::DataMap::Clear() {
    mDataMap.clear();
}