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

            template<typename T>
            std::string stringify_type(T const& tVal) {
                return std::to_string(tVal);
            }

            inline std::string stringify_type(std::string const& tVal) {
                return tVal;
            }

            //////////////////////////////////////////////////
            ///// SettingElement                         /////
            //////////////////////////////////////////////////

            template<typename Type>
            Type SettingElement::read_setting(std::string const& a_strFile, std::string const& a_strElement, Type const& a_tDefault) {
                // Request the boost::property_tree containing the current setting element
                // The given tree is automatically locked & read on request and written & unlocked on deletion
                if (auto const& pTree = get_tree(a_strFile, a_strElement, true)) {
                    // Get the key that points to where the data is stored in the tree
                    auto strKey = get_element(a_strFile, a_strElement)->get_key_m();
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
                if (auto const& pTree = get_tree(a_strFile, a_strElement, false)) {
                    // Get the key that points to where the data is stored in the tree
                    auto strKey = get_element(a_strFile, a_strElement)->get_key_m();
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
                    if (auto const& pTree = get_tree(Element::File::Name, Element::Name, false)) {
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
                    if (auto const& pTree = get_tree(Element::File::Name, Element::Name, true)) {
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
                    pElm->link_variable_m(
                        [&a_rtVariable] { a_rtVariable = Element::read(); },
                        [&a_rtVariable] { Element::write(a_rtVariable); }
                    );
                }
            }

            template<typename Type>
            std::vector<Type> SettingElement::read_setting_vector(std::string const& a_strFile, std::string const& a_strElement, std::vector<Type> const& a_tvecDefault) {
                std::vector<Type> vecOutput{};
                // Request the boost::property_tree containing the current setting element
                // The given tree is automatically locked & read on request and written & unlocked on deletion
                if (auto const& pTree = get_tree(a_strFile, a_strElement, true)) {
                    // Get the key that points to where the data is stored in the tree
                    auto strKey = get_element(a_strFile, a_strElement)->get_key_m();
                    // Get each the subtree corresponding to the key
                    try {
                        for(auto const& subTree : pTree->get_child(strKey)) {
                            // Read subTree content and add it to the vector
                            vecOutput.push_back(read_tree(subTree.second, Type{}));
                        }
                    }
                    catch(boost::property_tree::ptree_bad_path&) {
                        // get_child() may throw if the key does not exist
                        vecOutput = a_tvecDefault;
                    }
                }
                return vecOutput;
            }

            template<typename Type>
            void SettingElement::write_setting_vector(std::string const& a_strFile, std::string const& a_strElement, std::vector<Type> const& a_tvecNew) {
                // Request the boost::property_tree containing the current setting element
                // The given tree is automatically locked & read on request and written & unlocked on deletion
                if (auto const& pTree = get_tree(a_strFile, a_strElement, false)) {
                    // Get the key that points to where the data is stored in the tree
                    auto strKey = get_element(a_strFile, a_strElement)->get_key_m();
                    // Get the file type to customize data representation
                    auto eType = emb::settings::get_file(a_strFile)->get_type_m();
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
                if (auto const& pTree = get_tree(a_strFile, a_strElement, false)) {
                    // Get the key that points to where the data is stored in the tree
                    auto strKey = get_element(a_strFile, a_strElement)->get_key_m();
                    // Get the file type to customize data representation
                    auto eType = emb::settings::get_file(a_strFile)->get_type_m();
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

            template<typename Element>
            void SettingElement::reset_setting_vector() {
                switch (default_mode()) {
                case DefaultMode::DefaultValueIfAbsentFromFile:
                    // Request the boost::property_tree containing the current setting element
                    // The given tree is automatically locked & read on request and written & unlocked on deletion
                    if (auto const& pTree = get_tree(Element::File::Name, Element::Name, false)) {
                        // Remove the element from the tree
                        remove_tree(*pTree, Element::Key);
                    }
                    break;
                case DefaultMode::DefaultValueWrittenInFile:
                    write_setting_vector<typename Element::Type::value_type>(Element::File::Name, Element::Name, Element::Default);
                    break;
                }
            }

            template<typename Element>
            bool SettingElement::is_default_setting_vector() {
                bool bRes{ false };
                switch (default_mode()) {
                case DefaultMode::DefaultValueIfAbsentFromFile:
                    // Request the boost::property_tree containing the current setting element
                    // The given tree is automatically locked & read on request and written & unlocked on deletion
                    if (auto const& pTree = get_tree(Element::File::Name, Element::Name, true)) {
                        try {
                            (void)pTree->get_child(Element::Key);
                        }
                        catch (boost::property_tree::ptree_bad_path&) {
                            // get_child() may throw if the key does not exist
                            bRes = true;
                        }
                    }
                    break;
                case DefaultMode::DefaultValueWrittenInFile:
                    bRes = Element::Default == read_setting_vector<typename Element::Type::value_type>(Element::File::Name, Element::Name, Element::Default);
                    break;
                }
                return bRes;
            }

            template<typename Type>
            std::map<std::string, Type> SettingElement::read_setting_map(std::string const& a_strFile, std::string const& a_strElement, std::map<std::string, Type> const& a_tmapDefault) {
                std::map<std::string, Type> mapOutput{};
                // Request the boost::property_tree containing the current setting element
                // The given tree is automatically locked & read on request and written & unlocked on deletion
                if (auto const& pTree = get_tree(a_strFile, a_strElement, true)) {
                    // Get the key that points to where the data is stored in the tree
                    auto strKey = get_element(a_strFile, a_strElement)->get_key_m();
                    // Get each the subtree corresponding to the key
                    try {
                        for(auto const& subTree : pTree->get_child(strKey)) {
                            // Read subTree content and add it to the map
                            mapOutput[subTree.first] = read_tree(subTree.second, Type{});
                        }
                    }
                    catch(boost::property_tree::ptree_bad_path&) {
                        // get_child() may throw if the key does not exist
                        mapOutput = a_tmapDefault;
                    }
                }
                return mapOutput;
            }

            template<typename Type>
            void SettingElement::write_setting_map(std::string const& a_strFile, std::string const& a_strElement, std::map<std::string, Type> const& a_tmapNew) {
                // Request the boost::property_tree containing the current setting element
                // The given tree is automatically locked & read on request and written & unlocked on deletion
                if (auto const& pTree = get_tree(a_strFile, a_strElement, false)) {
                    // Get the key that points to where the data is stored in the tree
                    auto strKey = get_element(a_strFile, a_strElement)->get_key_m();
                    // Get the file type to customize data representation
                    auto eType = emb::settings::get_file(a_strFile)->get_type_m();
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
                if (auto const& pTree = get_tree(a_strFile, a_strElement, false)) {
                    // Get the key that points to where the data is stored in the tree
                    auto strKey = get_element(a_strFile, a_strElement)->get_key_m();
                    // Get the file type to customize data representation
                    auto eType = emb::settings::get_file(a_strFile)->get_type_m();
                    // Create and set the subtree accordingly to the file type
                    switch(eType) {
                    case FileType::XML:
                    case FileType::JSON:
                        if(auto subTree = get_sub_tree(pTree, strKey, true)) {
                            // Create a new subtree
                            boost::property_tree::ptree valTree{};
                            // Write the subtree
                            write_tree(valTree, a_tNew);
                            // Write the subtree into the main tree
                            subTree->put_child(boost::property_tree::ptree::path_type(a_strK, '\0'), valTree);
                        }
                        break;
                    case FileType::INI:
                        /// @todo
                        break;
                    }
                }
            }

            template<typename Element>
            void SettingElement::reset_setting_map() {
                switch (default_mode()) {
                case DefaultMode::DefaultValueIfAbsentFromFile:
                    // Request the boost::property_tree containing the current setting element
                    // The given tree is automatically locked & read on request and written & unlocked on deletion
                    if (auto const& pTree = get_tree(Element::File::Name, Element::Name, false)) {
                        // Remove the element from the tree
                        remove_tree(*pTree, Element::Key);
                    }
                    break;
                case DefaultMode::DefaultValueWrittenInFile:
                    write_setting_map<typename Element::Type::mapped_type>(Element::File::Name, Element::Name, Element::Default);
                    break;
                }
            }

            template<typename Element>
            bool SettingElement::is_default_setting_map() {
                bool bRes{ false };
                switch (default_mode()) {
                case DefaultMode::DefaultValueIfAbsentFromFile:
                    // Request the boost::property_tree containing the current setting element
                    // The given tree is automatically locked & read on request and written & unlocked on deletion
                    if (auto const& pTree = get_tree(Element::File::Name, Element::Name, true)) {
                        try {
                            (void)pTree->get_child(Element::Key);
                        }
                        catch (boost::property_tree::ptree_bad_path&) {
                            // get_child() may throw if the key does not exist
                            bRes = true;
                        }
                    }
                    break;
                case DefaultMode::DefaultValueWrittenInFile:
                    bRes = Element::Default == read_setting_map<typename Element::Type::mapped_type>(Element::File::Name, Element::Name, Element::Default);
                    break;
                }
                return bRes;
            }

            //////////////////////////////////////////////////
            ///// TSettingScalar                         /////
            //////////////////////////////////////////////////

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, _Type const* _Default>
            TSettingScalar<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::TSettingScalar()
                : SettingElement{ _NameStr, _TypeStr, _File::Name, _KeyStr }
            {}

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, _Type const* _Default>
            TSettingScalar<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::~TSettingScalar() {
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, _Type const* _Default>
            _Type TSettingScalar<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::read() {
                return read_setting<_Type>(_File::Name, _NameStr, Default);
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, _Type const* _Default>
            void TSettingScalar<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::write(_Type const& a_tVal) {
                write_setting<_Type>(_File::Name, _NameStr, a_tVal);
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, _Type const* _Default>
            void TSettingScalar<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::reset() {
                reset_setting<_Name>();
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, _Type const* _Default>
            bool TSettingScalar<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::is_default() {
                return is_default_setting<_Name>();
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, _Type const* _Default>
            void TSettingScalar<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::link(_Type& a_rtVar) {
                link_setting<_Type, _Name>(_File::Name, _NameStr, a_rtVar);
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, _Type const* _Default>
            std::string TSettingScalar<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::read_str_m() const {
                return read_setting<std::string>(_File::Name, _NameStr, stringify_type(Default));
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, _Type const* _Default>
            bool TSettingScalar<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::is_default_m() const {
                return is_default();
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, _Type const* _Default>
            void TSettingScalar<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::reset_m() const {
                reset();
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, _Type const* _Default>
            std::unique_ptr<SettingElement> TSettingScalar<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::_create_() {
                return std::make_unique<_Name>();
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, _Type const* _Default>
            bool TSettingScalar<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::s_bRegistered =
                SettingElement::register_element(_File::Name, _NameStr, _Name::_create_);
            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, _Type const* _Default>
            char const* TSettingScalar<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::Name{ _NameStr };
            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, _Type const* _Default>
            char const* TSettingScalar<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::Key{ _KeyStr };
            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, _Type const* _Default>
            _Type const TSettingScalar<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::Default{ _Default ? *_Default : _Type{} };

            //////////////////////////////////////////////////
            ///// TSettingVector                         /////
            //////////////////////////////////////////////////

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::vector<_Type> const* _Default>
            TSettingVector<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::TSettingVector()
                    : SettingElement{ _NameStr, _TypeStr, _File::Name, _KeyStr }
            {}

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::vector<_Type> const* _Default>
            TSettingVector<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::~TSettingVector() {
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::vector<_Type> const* _Default>
            std::vector<_Type> TSettingVector<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::read() {
                return read_setting_vector<_Type>(_File::Name, _NameStr, Default);
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::vector<_Type> const* _Default>
            void TSettingVector<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::write(std::vector<_Type> const& a_tvecVal) {
                write_setting_vector<_Type>(_File::Name, _NameStr, a_tvecVal);
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::vector<_Type> const* _Default>
            void TSettingVector<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::add(_Type const& a_tVal) {
                add_setting_vector<_Type>(_File::Name, _NameStr, a_tVal);
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::vector<_Type> const* _Default>
            void TSettingVector<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::reset() {
                reset_setting_vector<_Name>();
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::vector<_Type> const* _Default>
            bool TSettingVector<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::is_default() {
                return is_default_setting_vector<_Name>();
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::vector<_Type> const* _Default>
            void TSettingVector<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::link(std::vector<_Type>& a_rtvecVal) {
                link_setting<std::vector<_Type>, _Name>(_File::Name, _NameStr, a_rtvecVal);
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::vector<_Type> const* _Default>
            std::string TSettingVector<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::read_str_m() const {
                // Request the boost::property_tree containing the current setting element
                // The given tree is automatically locked & read on request and written & unlocked on deletion
                if (auto const& pTree = get_tree(_File::Name, _NameStr, true)) {
                    // Get each the subtree corresponding to the key
                    try {
                        boost::property_tree::ptree newTree{};
                        for (auto const& subTree : pTree->get_child(_KeyStr)) {
                            // Read subTree content and add it to the new tree
                            newTree.push_back(std::make_pair("", subTree.second));
                        }
                        boost::property_tree::ptree newRootTree{};
                        newRootTree.add_child("root", newTree);
                        return stringify_tree(newRootTree);
                    }
                    catch (boost::property_tree::ptree_bad_path&) {
                        // get_child() may throw if the key does not exist
                    }
                }
                return "[?]";
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::vector<_Type> const* _Default>
            bool TSettingVector<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::is_default_m() const {
                return is_default();
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::vector<_Type> const* _Default>
            void TSettingVector<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::reset_m() const {
                reset();
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::vector<_Type> const* _Default>
            std::unique_ptr<SettingElement> TSettingVector<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::_create_() {
                return std::make_unique<_Name>();
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::vector<_Type> const* _Default>
            bool TSettingVector<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::s_bRegistered =
                SettingElement::register_element(_File::Name, _NameStr, _Name::_create_);
            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::vector<_Type> const* _Default>
            char const* TSettingVector<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::Name{ _NameStr };
            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::vector<_Type> const* _Default>
            char const* TSettingVector<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::Key{ _KeyStr };
            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::vector<_Type> const* _Default>
            std::vector<_Type> const TSettingVector<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::Default{ _Default ? *_Default : std::vector<_Type>{} };

            //////////////////////////////////////////////////
            ///// TSettingMap                            /////
            //////////////////////////////////////////////////

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::map<std::string, _Type> const* _Default>
            TSettingMap<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::TSettingMap()
                    : SettingElement{ _NameStr, _TypeStr, _File::Name, _KeyStr }
            {}

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::map<std::string, _Type> const* _Default>
            TSettingMap<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::~TSettingMap() {
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::map<std::string, _Type> const* _Default>
            std::map<std::string, _Type> TSettingMap<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::read() {
                return read_setting_map<_Type>(_File::Name, _NameStr, Default);
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::map<std::string, _Type> const* _Default>
            void TSettingMap<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::write(std::map<std::string, _Type> const& a_tmapVal) {
                write_setting_map<_Type>(_File::Name, _NameStr, a_tmapVal);
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::map<std::string, _Type> const* _Default>
            void TSettingMap<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::set(std::string const& a_strKey, _Type const& a_tVal) {
                set_setting_map<_Type>(_File::Name, _NameStr, a_strKey, a_tVal);
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::map<std::string, _Type> const* _Default>
            void TSettingMap<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::reset() {
                reset_setting_map<_Name>();
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::map<std::string, _Type> const* _Default>
            bool TSettingMap<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::is_default() {
                return is_default_setting_map<_Name>();
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::map<std::string, _Type> const* _Default>
            void TSettingMap<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::link(std::map<std::string, _Type>& a_rtmapVal) {
                link_setting<std::map<std::string, _Type>, _Name>(_File::Name, _NameStr, a_rtmapVal);
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::map<std::string, _Type> const* _Default>
            std::string TSettingMap<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::read_str_m() const {
                // Request the boost::property_tree containing the current setting element
                // The given tree is automatically locked & read on request and written & unlocked on deletion
                if (auto const& pTree = get_tree(_File::Name, _NameStr, true)) {
                    // Get each the subtree corresponding to the key
                    try {
                        boost::property_tree::ptree newTree{};
                        for (auto const& subTree : pTree->get_child(_KeyStr)) {
                            // Read subTree content and add it to the new tree
                            newTree.put_child(subTree.first, subTree.second);
                        }
                        return stringify_tree(newTree);
                    }
                    catch (boost::property_tree::ptree_bad_path&) {
                        // get_child() may throw if the key does not exist
                    }
                }
                return "{?}";
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::map<std::string, _Type> const* _Default>
            bool TSettingMap<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::is_default_m() const {
                return is_default();
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::map<std::string, _Type> const* _Default>
            void TSettingMap<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::reset_m() const {
                reset();
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::map<std::string, _Type> const* _Default>
            std::unique_ptr<SettingElement> TSettingMap<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::_create_() {
                return std::make_unique<_Name>();
            }

            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::map<std::string, _Type> const* _Default>
            bool TSettingMap<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::s_bRegistered =
                SettingElement::register_element(_File::Name, _NameStr, _Name::_create_);
            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::map<std::string, _Type> const* _Default>
            char const* TSettingMap<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::Name{ _NameStr };
            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::map<std::string, _Type> const* _Default>
            char const* TSettingMap<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::Key{ _KeyStr };
            template<typename _Name, char const* _NameStr, typename _Type, char const* _TypeStr, typename _File, char const* _KeyStr, std::map<std::string, _Type> const* _Default>
            std::map<std::string, _Type> const TSettingMap<_Name, _NameStr, _Type, _TypeStr, _File, _KeyStr, _Default>::Default{ _Default ? *_Default : std::map<std::string, _Type>{} };

            //////////////////////////////////////////////////
            ///// TSettingsFile                          /////
            //////////////////////////////////////////////////

            template<typename _Name, char const* _NameStr, emb::settings::FileType _Type, char const* _PathStr, int _Version, version_clbk_t _VersionClbk>
            TSettingsFile<_Name, _NameStr, _Type, _PathStr, _Version, _VersionClbk>::TSettingsFile()
                    : SettingsFile{ _NameStr, _Type, _PathStr, _Version, _VersionClbk }
            {}

            template<typename _Name, char const* _NameStr, emb::settings::FileType _Type, char const* _PathStr, int _Version, version_clbk_t _VersionClbk>
            TSettingsFile<_Name, _NameStr, _Type, _PathStr, _Version, _VersionClbk>::~TSettingsFile() {
            }

            template<typename _Name, char const* _NameStr, emb::settings::FileType _Type, char const* _PathStr, int _Version, version_clbk_t _VersionClbk>
            void TSettingsFile<_Name, _NameStr, _Type, _PathStr, _Version, _VersionClbk>::begin() {
                begin_file_transaction(_NameStr);
            }

            template<typename _Name, char const* _NameStr, emb::settings::FileType _Type, char const* _PathStr, int _Version, version_clbk_t _VersionClbk>
            void TSettingsFile<_Name, _NameStr, _Type, _PathStr, _Version, _VersionClbk>::commit() {
                commit_file_transaction(_NameStr);
            }

            template<typename _Name, char const* _NameStr, emb::settings::FileType _Type, char const* _PathStr, int _Version, version_clbk_t _VersionClbk>
            void TSettingsFile<_Name, _NameStr, _Type, _PathStr, _Version, _VersionClbk>::abort() {
                abort_file_transaction(_NameStr);
            }

            template<typename _Name, char const* _NameStr, emb::settings::FileType _Type, char const* _PathStr, int _Version, version_clbk_t _VersionClbk>
            void TSettingsFile<_Name, _NameStr, _Type, _PathStr, _Version, _VersionClbk>::read_linked() {
                for(auto const& elm: get_element_names_list(_NameStr)) {
                    if(auto const& pElm = get_element(_NameStr, elm)) {
                        pElm->read_linked_m();
                    }
                }
            }

            template<typename _Name, char const* _NameStr, emb::settings::FileType _Type, char const* _PathStr, int _Version, version_clbk_t _VersionClbk>
            void TSettingsFile<_Name, _NameStr, _Type, _PathStr, _Version, _VersionClbk>::write_linked() {
                for(auto const& elm: get_element_names_list(_NameStr)) {
                    if(auto const& pElm = get_element(_NameStr, elm)) {
                        pElm->write_linked_m();
                    }
                }
            }

            template<typename _Name, char const* _NameStr, emb::settings::FileType _Type, char const* _PathStr, int _Version, version_clbk_t _VersionClbk>
            bool TSettingsFile<_Name, _NameStr, _Type, _PathStr, _Version, _VersionClbk>::backup_to(std::string const& a_strFolderName) {
                return backup_file(_NameStr, a_strFolderName);
            }

            template<typename _Name, char const* _NameStr, emb::settings::FileType _Type, char const* _PathStr, int _Version, version_clbk_t _VersionClbk>
            bool TSettingsFile<_Name, _NameStr, _Type, _PathStr, _Version, _VersionClbk>::restore_from(std::string const& a_strFolderName) {
                return restore_file(_NameStr, a_strFolderName);
            }

            template<typename _Name, char const* _NameStr, emb::settings::FileType _Type, char const* _PathStr, int _Version, version_clbk_t _VersionClbk>
            std::unique_ptr<SettingsFile> TSettingsFile<_Name, _NameStr, _Type, _PathStr, _Version, _VersionClbk>::_create_() {
                return std::make_unique<_Name>();
            }

            template<typename _Name, char const* _NameStr, FileType _Type, char const* _PathStr, int _Version, version_clbk_t _VersionClbk>
            bool TSettingsFile<_Name, _NameStr, _Type, _PathStr, _Version, _VersionClbk>::s_bRegistered = SettingsFile::register_file(_NameStr, _Name::_create_);
            template<typename _Name, char const* _NameStr, FileType _Type, char const* _PathStr, int _Version, version_clbk_t _VersionClbk>
            char const* TSettingsFile<_Name, _NameStr, _Type, _PathStr, _Version, _VersionClbk>::Name{ _NameStr };
            template<typename _Name, char const* _NameStr, FileType _Type, char const* _PathStr, int _Version, version_clbk_t _VersionClbk>
            char const* TSettingsFile<_Name, _NameStr, _Type, _PathStr, _Version, _VersionClbk>::Path{ _PathStr };
            template<typename _Name, char const* _NameStr, FileType _Type, char const* _PathStr, int _Version, version_clbk_t _VersionClbk>
            emb::settings::FileType const TSettingsFile<_Name, _NameStr, _Type, _PathStr, _Version, _VersionClbk>::Type{ _Type };
            template<typename _Name, char const* _NameStr, FileType _Type, char const* _PathStr, int _Version, version_clbk_t _VersionClbk>
            int const TSettingsFile<_Name, _NameStr, _Type, _PathStr, _Version, _VersionClbk>::Version{ _Version };
            template<typename _Name, char const* _NameStr, FileType _Type, char const* _PathStr, int _Version, version_clbk_t _VersionClbk>
            version_clbk_t const TSettingsFile<_Name, _NameStr, _Type, _PathStr, _Version, _VersionClbk>::VersionClbk{ _VersionClbk };

        }
    }
}
