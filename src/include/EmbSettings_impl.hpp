#pragma once
//#define DEBUG_REGISTER
#include "EmbSettings.hpp"
#include <string>
#ifdef DEBUG_REGISTER
#include <iostream>
#endif


namespace emb {
    namespace settings {
        namespace internal {

            //////////////////////////////////////////////////
            ///// Default read/write methods             /////
            //////////////////////////////////////////////////

            template<typename T>
            T read_tree(boost::property_tree::ptree const& a_rTree, T const& tDefaultVal) {
                return a_rTree.get<T>("", tDefaultVal);
            }

            template<typename T>
            void write_tree(boost::property_tree::ptree & a_rTree, T const& tVal) {
                a_rTree.put<T>("", tVal);
            }

            //////////////////////////////////////////////////
            ///// SettingElement                         /////
            //////////////////////////////////////////////////

            template<typename Type>
            Type SettingElement::read_setting(std::string const& a_strFile, std::string const& a_strElement, Type const& a_tDefault) {
                // Request the boost::property_tree containing the current setting element
                // The given tree is automatically locked & read on request and written & unlocked on deletion
                if (auto const& pTree = get_tree(a_strFile, a_strElement)) {
                    // Get the key that points to where the data is stored in the tree
                    auto strKey = get_element(a_strFile, a_strElement)->get_key();
                    // Get the subtree corresponding to the key
                    if(auto const& subTree = pTree->get_child_optional(strKey)) {
                        return read_tree(*subTree, a_tDefault);
                    }
                }
                return a_tDefault;
            }

            template<typename Type>
            void SettingElement::write_setting(std::string const& a_strFile, std::string const& a_strElement, Type const& a_tNew) {
                // Request the boost::property_tree containing the current setting element
                // The given tree is automatically locked & read on request and written & unlocked on deletion
                if (auto const& pTree = get_tree(a_strFile, a_strElement)) {
                    // Get the key that points to where the data is stored in the tree
                    auto strKey = get_element(a_strFile, a_strElement)->get_key();
                    // Get the subtree pointed by the key or a new tree if it does not exist
                    boost::property_tree::ptree subTree{};
                    auto& rSubTree = pTree->get_child(strKey, subTree);
                    // Write the subtree
                    write_tree(rSubTree, a_tNew);
                    // If it is a new subtree, it needs to be written in the main tree
                    if(rSubTree == subTree) {
                        pTree->add_child(strKey, subTree);
                    }
                }
            }

            template<typename Element>
            void SettingElement::reset_setting() {
                switch(default_mode()) {
                case DefaultMode::DefaultValueIfAbsentFromFile:
                    // Request the boost::property_tree containing the current setting element
                    // The given tree is automatically locked & read on request and written & unlocked on deletion
                    if (auto const& pTree = get_tree(Element::File::Name, Element::Name)) {
                        // Remove the element from the tree
                        remove_tree(*pTree, Element::Key);
                    }
                    break;
                case DefaultMode::DefaultValueWrittenInFile:
                    write_setting<typename Element::Type>(Element::File::Name, Element::Name, Element::Default);
                    break;
                }
            }

            template<typename Element>
            bool SettingElement::is_default_setting() {
                bool bRes{false};
                switch(default_mode()) {
                case DefaultMode::DefaultValueIfAbsentFromFile:
                    // Request the boost::property_tree containing the current setting element
                    // The given tree is automatically locked & read on request and written & unlocked on deletion
                    if (auto const& pTree = get_tree(Element::File::Name, Element::Name)) {
                        try {
                            (void)pTree->get_child(Element::Key);
                        }
                        catch(boost::property_tree::ptree_bad_path&) {
                            // get_child() may throw if the key does not exist
                            bRes = true;
                        }
                    }
                    break;
                case DefaultMode::DefaultValueWrittenInFile:
                    bRes = Element::Default == read_setting<typename Element::Type>(Element::File::Name, Element::Name, Element::Default);
                    break;
                }
                return bRes;
            }

            template<typename Type, typename Element>
            void SettingElement::link_setting(std::string const& a_strFile, std::string const& a_strElement, Type& a_rtVariable) {
                if(auto const& pElm = get_element(a_strFile, a_strElement)) {
                    pElm->link_variable(
                        [&a_rtVariable] { a_rtVariable = Element::read(); },
                        [&a_rtVariable] { Element::write(a_rtVariable); }
                    );
                }
            }

            template<typename Type>
            std::vector<Type> SettingElement::read_setting_vector(std::string const& a_strFile, std::string const& a_strElement) {
                std::vector<Type> vecOutput{};
                // Request the boost::property_tree containing the current setting element
                // The given tree is automatically locked & read on request and written & unlocked on deletion
                if (auto const& pTree = get_tree(a_strFile, a_strElement)) {
                    // Get the key that points to where the data is stored in the tree
                    auto strKey = get_element(a_strFile, a_strElement)->get_key();
                    // Get each the subtree corresponding to the key
                    try {
                        for(auto const& subTree : pTree->get_child(strKey)) {
                            // Read subTree content and add it to the vector
                            vecOutput.push_back(read_tree(subTree.second, Type{}));
                        }
                    }
                    catch(boost::property_tree::ptree_bad_path&) {
                        // get_child() may throw if the key does not exist
                    }
                }
                return vecOutput;
            }

            template<typename Type>
            void SettingElement::write_setting_vector(std::string const& a_strFile, std::string const& a_strElement, std::vector<Type> const& a_tvecNew) {
                // Request the boost::property_tree containing the current setting element
                // The given tree is automatically locked & read on request and written & unlocked on deletion
                if (auto const& pTree = get_tree(a_strFile, a_strElement)) {
                    // Get the key that points to where the data is stored in the tree
                    auto strKey = get_element(a_strFile, a_strElement)->get_key();
                    // Get the file type to customize data representation
                    auto eType = emb::settings::get_file(a_strFile)->get_type();
                    // Remove old subtree
                    remove_tree(*pTree, strKey);
                    // Create and add the subtree accordingly to the file type
                    switch(eType) {
                    case FileType::XML:
                        for(auto const& value : a_tvecNew) {
                            // Create a new subtree
                            boost::property_tree::ptree subTree{};
                            // Write the subtree
                            write_tree(subTree, value);
                            // Write the subtree into the main tree
                            pTree->add_child(strKey + "." + internal::xml_vector_element_name(), subTree);
                        }
                        break;
                    case FileType::JSON: {
                            boost::property_tree::ptree children;
                            for(auto const& value : a_tvecNew) {
                                // Create a new subtree
                                boost::property_tree::ptree subTree{};
                                // Write the subtree
                                write_tree(subTree, value);
                                // Write the subtree into the main tree
                                children.push_back(std::make_pair("", subTree));
                            }
                            pTree->add_child(strKey, children);
                        }
                        break;
                    case FileType::INI:
                        /// @todo
                        break;
                    }
                }
            }

            template<typename Type>
            void SettingElement::add_setting_vector(std::string const& a_strFile, std::string const& a_strElement, Type const& a_tNew) {
                // Request the boost::property_tree containing the current setting element
                // The given tree is automatically locked & read on request and written & unlocked on deletion
                if (auto const& pTree = get_tree(a_strFile, a_strElement)) {
                    // Get the key that points to where the data is stored in the tree
                    auto strKey = get_element(a_strFile, a_strElement)->get_key();
                    // Get the file type to customize data representation
                    auto eType = emb::settings::get_file(a_strFile)->get_type();
                    // Create and add the subtree accordingly to the file type
                    switch(eType) {
                    case FileType::XML: {
                            // Create a new subtree
                            boost::property_tree::ptree subTree{};
                            // Write the subtree
                            write_tree(subTree, a_tNew);
                            // Write the subtree into the main tree
                            pTree->add_child(strKey + "." + internal::xml_vector_element_name(), subTree);
                        }
                        break;
                    case FileType::JSON: {
                            // Create a new subtree
                            boost::property_tree::ptree subTree;
                            // Write the subtree
                            write_tree(subTree, a_tNew);
                            // Write the subtree into the main tree
                            pTree->get_child(strKey).push_back(std::make_pair("", subTree));
                        }
                        break;
                    case FileType::INI:
                        break;
                    }
                }
            }

            template<typename Type>
            std::map<std::string, Type> SettingElement::read_setting_map(std::string const& a_strFile, std::string const& a_strElement) {
                std::map<std::string, Type> mapOutput{};
                // Request the boost::property_tree containing the current setting element
                // The given tree is automatically locked & read on request and written & unlocked on deletion
                if (auto const& pTree = get_tree(a_strFile, a_strElement)) {
                    // Get the key that points to where the data is stored in the tree
                    auto strKey = get_element(a_strFile, a_strElement)->get_key();
                    // Get each the subtree corresponding to the key
                    try {
                        for(auto const& subTree : pTree->get_child(strKey)) {
                            // Read subTree content and add it to the map
                            mapOutput[subTree.first] = read_tree(subTree.second, Type{});
                        }
                    }
                    catch(boost::property_tree::ptree_bad_path&) {
                        // get_child() may throw if the key does not exist
                    }
                }
                return mapOutput;
            }

            template<typename Type>
            void SettingElement::write_setting_map(std::string const& a_strFile, std::string const& a_strElement, std::map<std::string, Type> const& a_tmapNew) {
                // Request the boost::property_tree containing the current setting element
                // The given tree is automatically locked & read on request and written & unlocked on deletion
                if (auto const& pTree = get_tree(a_strFile, a_strElement)) {
                    // Get the key that points to where the data is stored in the tree
                    auto strKey = get_element(a_strFile, a_strElement)->get_key();
                    // Get the file type to customize data representation
                    auto eType = emb::settings::get_file(a_strFile)->get_type();
                    // Remove old subtree
                    remove_tree(*pTree, strKey);
                    // Create and add the subtree accordingly to the file type
                    switch(eType) {
                    case FileType::XML:
                    case FileType::JSON:
                        for(auto const& value : a_tmapNew) {
                            // Create a new subtree
                            boost::property_tree::ptree subTree{};
                            // Write the subtree
                            write_tree(subTree, value.second);
                            // Write the subtree into the main tree
                            pTree->add_child(strKey + "." + value.first, subTree);
                        }
                        break;
                    case FileType::INI:
                        /// @todo
                        break;
                    }
                }
            }

            template<typename Type>
            void SettingElement::set_setting_map(std::string const& a_strFile, std::string const& a_strElement, std::string const& a_strK, Type const& a_tNew) {
                // Request the boost::property_tree containing the current setting element
                // The given tree is automatically locked & read on request and written & unlocked on deletion
                if (auto const& pTree = get_tree(a_strFile, a_strElement)) {
                    // Get the key that points to where the data is stored in the tree
                    auto strKey = get_element(a_strFile, a_strElement)->get_key();
                    // Get the file type to customize data representation
                    auto eType = emb::settings::get_file(a_strFile)->get_type();
                    // Create and set the subtree accordingly to the file type
                    switch(eType) {
                    case FileType::XML:
                    case FileType::JSON: {
                            // Create a new subtree
                            boost::property_tree::ptree subTree{};
                            // Write the subtree
                            write_tree(subTree, a_tNew);
                            // Write the subtree into the main tree
                            pTree->put_child(strKey + "." + a_strK, subTree);
                        }
                        break;
                    case FileType::INI:
                        /// @todo
                        break;
                    }
                }
            }


            template<typename _Name, char const* _NameStr, emb::settings::FileType _Type, char const* _PathStr, int _Version>
            void TSettingsFile<_Name, _NameStr, _Type, _PathStr, _Version>::read_linked() {
                for(auto const& elm: get_element_names_list(_NameStr)) {
                    if(auto const& pElm = get_element(_NameStr, elm)) {
                        pElm->read_linked();
                    }
                }
            }

            template<typename _Name, char const* _NameStr, emb::settings::FileType _Type, char const* _PathStr, int _Version>
            void TSettingsFile<_Name, _NameStr, _Type, _PathStr, _Version>::write_linked() {
                for(auto const& elm: get_element_names_list(_NameStr)) {
                    if(auto const& pElm = get_element(_NameStr, elm)) {
                        pElm->write_linked();
                    }
                }
            }

        }
    }
}
