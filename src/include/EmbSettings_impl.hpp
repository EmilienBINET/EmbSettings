#pragma once
//#define DEBUG_REGISTER
#include "EmbSettings.hpp"
#include <string>
#ifdef DEBUG_REGISTER
#include <iostream>
#endif


namespace emb {
    namespace settings {

        template<typename T>
        T read_tree(boost::property_tree::ptree const& a_rTree, T const& tDefaultVal) {
            return a_rTree.get<T>("", tDefaultVal);
        }

        template<typename T>
        void write_tree(boost::property_tree::ptree & a_rTree, T const& tVal) {
            a_rTree.put<T>("", tVal);
        }

        template<typename Type>
        Type SettingsElement::read_setting(std::string const& a_strFileClass, std::string const& a_strKey, Type const& a_tDefaultValue) {
            if (auto const& fileInfo = getFilesMap()[a_strFileClass]) {
                if (auto pInfo = internal::SettingsFileInfo::getFileInfo((*fileInfo)())) {
                    // Get the child pointed by a_strKey
                    if(auto const& tree = pInfo->tree.get_child_optional(a_strKey)) {
                        return read_tree(*tree, a_tDefaultValue);
                    }
                }
            }
            return a_tDefaultValue;
        }

        template<typename Type>
        void SettingsElement::write_setting(std::string const& a_strFileClass, std::string const& a_strKey, Type const& a_tNewValue) {
            if (auto const& fileInfo = getFilesMap()[a_strFileClass]) {
                if (auto pInfo = internal::SettingsFileInfo::getFileInfo((*fileInfo)())) {
                    // Get the child pointed by a_strKey or tree if it does not exist
                    boost::property_tree::ptree tree{};
                    auto& rTree = pInfo->tree.get_child(a_strKey, tree);
                    // Write the subtree
                    write_tree(rTree, a_tNewValue);
                    // If it is a new subtree, it needs to be written in the main tree
                    if(rTree == tree) {
                        pInfo->tree.add_child(a_strKey, tree);
                    }
                }
            }
        }

        template<typename Type, typename Element>
        void SettingsElement::link_setting(std::string const& a_strFileClass, std::string const& a_strKey, Type & a_rtValue) {
            SettingsElement::getLinks()[a_strKey] = std::make_pair([&a_rtValue]{ a_rtValue = Element::read(); }, [&a_rtValue]{ Element::write(a_rtValue); });
        }

        template<typename Type>
        std::vector<Type> SettingsElement::read_setting_vector(std::string const& a_strFileClass, std::string const& a_strKey) {
            std::vector<Type> vecOutput{};
            if (auto const& fileInfo = getFilesMap()[a_strFileClass]) {
                if (auto pInfo = internal::SettingsFileInfo::getFileInfo((*fileInfo)())) {
                    try {
                        for(auto const& item : pInfo->tree.get_child(a_strKey)) {
                            vecOutput.push_back(read_tree(item.second, Type{}));
                        }
                    }
                    catch(...) {}
                }
            }
            return vecOutput;
        }

        template<typename Type>
        void SettingsElement::write_setting_vector(std::string const& a_strFileClass, std::string const& a_strKey, std::vector<Type> const& a_tvecNewValue) {
            if (auto const& fileInfo = getFilesMap()[a_strFileClass]) {
                if (auto pInfo = internal::SettingsFileInfo::getFileInfo((*fileInfo)())) {
                    auto pos = a_strKey.find_last_of('.');
                    if(std::string::npos != pos) {
                        pInfo->tree.get_child(a_strKey.substr(0, pos)).erase(a_strKey.substr(pos+1));
                    }
                    switch(pInfo->eFileType) {
                    case FileType::XML: {
                            for(auto const& item : a_tvecNewValue) {
                                // Create a new subtree
                                boost::property_tree::ptree tree{};
                                // Write the subtree
                                write_tree(tree, item);
                                // Write the subtree into the main tree
                                pInfo->tree.add_child(a_strKey + "." + internal::xml_vector_element_name(), tree);
                            }
                        }
                        break;
                    case FileType::JSON: {
                            boost::property_tree::ptree children;
                            for(auto const& item : a_tvecNewValue) {
                                // Create a new subtree
                                boost::property_tree::ptree tree{};
                                // Write the subtree
                                write_tree(tree, item);
                                // Write the subtree into the main tree
                                children.push_back(std::make_pair("", tree));
                            }
                            pInfo->tree.add_child(a_strKey, children);
                        }
                        break;
                    case FileType::INI: {
                            int iIdx{ 0 };
                            for (auto const& item : a_tvecNewValue) {
                                // Create a new subtree
                                boost::property_tree::ptree tree{};
                                // Write the subtree
                                write_tree(tree, item);
                                // Write the subtree into the main tree
                                std::string key{ a_strKey };
                                std::replace(key.begin(), key.end(), '.', '\\');
                                key += "." + std::to_string(iIdx++);
                                pInfo->tree.add_child(key, tree);
                            }
                        }
                        break;
                    }
                }
            }
        }

        template<typename Type>
        void SettingsElement::add_setting_vector(std::string const& a_strFileClass, std::string const& a_strKey, Type const& tVal) {
            if (auto const& fileInfo = getFilesMap()[a_strFileClass]) {
                if (auto pInfo = internal::SettingsFileInfo::getFileInfo((*fileInfo)())) {
                    switch(pInfo->eFileType) {
                    case FileType::XML: {
                            boost::property_tree::ptree child{};
                            write_tree(child, tVal);
                            pInfo->tree.add_child(a_strKey + "." + internal::xml_vector_element_name(), child);
                        }
                        break;
                    case FileType::JSON: {
                            boost::property_tree::ptree child;
                            write_tree(child, tVal);
                            pInfo->tree.get_child(a_strKey).push_back(std::make_pair("", child));
                        }
                        break;
                    case FileType::INI:
                        break;
                    }
                }
            }
        }

        template<typename Type>
        std::map<std::string, Type> SettingsElement::read_setting_map(std::string const& a_strFileClass, std::string const& a_strKey) {
            std::map<std::string, Type> mapOutput{};
            if (auto const& fileInfo = getFilesMap()[a_strFileClass]) {
                if (auto pInfo = internal::SettingsFileInfo::getFileInfo((*fileInfo)())) {
                    try {
                        for(auto const& item : pInfo->tree.get_child(a_strKey)) {
                            mapOutput[item.first] = read_tree(item.second, Type{});
                        }
                    }
                    catch(...) {
                    }
                }
            }
            return mapOutput;
        }

        template<typename Type>
        void SettingsElement::write_setting_map(std::string const& a_strFileClass, std::string const& a_strKey, std::map<std::string, Type> const& a_tmapNewValue) {
            if (auto const& fileInfo = getFilesMap()[a_strFileClass]) {
                if (auto pInfo = internal::SettingsFileInfo::getFileInfo((*fileInfo)())) {
                    auto pos = a_strKey.find_last_of('.');
                    if(std::string::npos != pos) {
                        try {
                            pInfo->tree.get_child(a_strKey.substr(0, pos)).erase(a_strKey.substr(pos+1));
                        }
                        catch(...) {
                        }
                    }
                    switch(pInfo->eFileType) {
                    case FileType::XML:
                    case FileType::JSON: {
                            for(auto const& item : a_tmapNewValue) {
                                // Create a new subtree
                                boost::property_tree::ptree tree{};
                                // Write the subtree
                                write_tree(tree, item.second);
                                // Write the subtree into the main tree
                                pInfo->tree.add_child(a_strKey + "." + item.first, tree);
                            }
                        }
                        break;
                    case FileType::INI:
                        break;
                    }
                }
            }
        }

        template<typename Type>
        void SettingsElement::set_setting_map(std::string const& a_strFileClass, std::string const& a_strKey, std::string const& a_strK, Type const& tVal) {
            if (auto const& fileInfo = getFilesMap()[a_strFileClass]) {
                if (auto pInfo = internal::SettingsFileInfo::getFileInfo((*fileInfo)())) {
                    switch(pInfo->eFileType) {
                    case FileType::XML:
                    case FileType::JSON: {
                            boost::property_tree::ptree child{};
                            write_tree(child, tVal);
                            pInfo->tree.put_child(a_strKey + "." + a_strK, child);
                        }
                        break;
                    case FileType::INI:
                        break;
                    }
                }
            }
        }

        template<typename Type>
        Type SettingsElement::read(Type const& a_tDefaultValue) const {
            return read_setting<Type>(getFileClassName(), getPath(), a_tDefaultValue);
        }
        template<typename Type>
        void SettingsElement::write(Type const& a_tNewValue) const {
            write_setting<Type>(getFileClassName(), getPath(), a_tNewValue);
        }

    }
}
