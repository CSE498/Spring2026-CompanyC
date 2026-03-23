/**
 * @file DataMap.hpp
 * @author Sehaj Gupta - Group 07
 * @brief A dynamic map that can match names to arbitrary types of data. Following Google style guide
 */

#pragma once
#include <unordered_map>
#include <string>
#include <any>
#include <expected>

namespace cse498 {

 class DataMap{
    // membver variables
    private:
        // any - to make it easier to store different types of data
        std::unordered_map<std::string, std::any>  mDataMap;

    public:
        // function to set data in the map
        void SetData(const  std::string& key, const std::any& value);

        /**
         * @brief Get data from the map with error handling using std::expected
         * @param key The key to look up in the map
         * @return std::expected<T, std::string> The value associated with the key if it exists and can be cast to the requested type, or an error message if the key is not found or the type does not match
         */
        template<typename T>
        std::expected<T, std::string> GetData(const std::string& key) const {
            auto it = mDataMap.find(key);
            if (it == mDataMap.end()) {
                return std::unexpected("Key not found: " + key);
            }
            
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
                return std::unexpected("Type mismatch for key: " + key);
            }
        };


        /**
         * @brief Check if the map contains a key
         * @param key The key to check for in the map
         * @return true if the key exists in the map, false otherwise
         */
        bool Contains(const std::string& key) const;

        /**
         * @brief Remove data from the map
         * @param key The key to remove from the map
         * @note If the key does not exist, this function will do nothing.
         */
        void RemoveData(const std::string& key);

        /**
         * @brief Clear all data from the map
         */
        void Clear();


 };

}