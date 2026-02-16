#include "DataMap.hpp"


void cse498::DataMap::SetData(const std::string& key, const std::any& value) {
    data_map[key] = value;
}


bool cse498::DataMap::Contains(const std::string& key) const {
    return data_map.find(key) != data_map.end();
}


void cse498::DataMap::RemoveData(const std::string& key) {
    data_map.erase(key);
}


void cse498::DataMap::Clear() {
    data_map.clear();
}