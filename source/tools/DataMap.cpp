#include "DataMap.hpp"


void DataMap::SetData(const std::string& key, const std::any& value) {
    data_map[key] = value;
}


bool DataMap::Contains(const std::string& key) const {
    return data_map.find(key) != data_map.end();
}


void DataMap::RemoveData(const std::string& key) {
    data_map.erase(key);
}


void DataMap::Clear() {
    data_map.clear();
}