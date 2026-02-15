/**
 * @file DataMap.hpp
 * @author Sehaj Gupta -Group 07
 * @brief A dynamic map that can match names to arbitrary types of data.
 */

# pragma once
# include <unordered_map>
# include <string>
# include <any>
# include <stdexcept>

 class DataMap{
    // membver variables
    private:
        // any - to make it easier to store different types of data
        std::unordered_map<std::string, std::any> data_map;

    public:
        // function to set data in the map
        void SetData(const  std::string& key, const std::any& value);

        // function to get data from the map
        template<typename T>
        T GetData(const std::string& key) const{
            auto it = data_map.find(key);
            if (it != data_map.end()) {
                return std::any_cast<T>(it->second);
            }
            throw std::runtime_error("Key not found in DataMap");
        }


        // function to check if a key exists in the map
        bool Contains(const std::string& key) const;

        // function to remove data from the map
        void RemoveData(const std::string& key);

        // function to clear the map
        void Clear();


 };