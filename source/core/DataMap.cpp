#include "DataMap.hpp"


void DataMap::SetData(const std::string& key, const std::any& value) {
    data_map[key] = value;
}


template<typename T>
T DataMap::GetData(const std::string& key) const {
    auto it = data_map.find(key);
    if (it != data_map.end()) {
        return std::any_cast<T>(it->second);
    }
    throw std::runtime_error("Key not found in DataMap");
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