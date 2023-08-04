#include "../include/EmbSettings.hpp"
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <mutex>
#include <map>
#include <regex>

//#define DEBUG_OUTPUT

namespace emb {
    namespace settings {
        using namespace std;

        void start() {
        }

        void stop() {
        }

        void setJocker(std::string const& aJocker, std::string const& aValue) {
        }

        SettingsElement::SettingsElement(std::string const& a_strClassName, std::string const& a_strType, std::string const& a_strFileClassName, std::string const& a_strPath)
                : m_strClassName{ a_strClassName }
                , m_strType{ a_strType }
                , m_strFileClassName{ a_strFileClassName }
                , m_strPath{ a_strPath }
        {}

        std::string SettingsElement::getClassName() const {
            return m_strClassName;
        }

        std::string SettingsElement::getType() const {
            return m_strType;
        }

        std::string SettingsElement::getFileClassName() const {
            return m_strFileClassName;
        }

        std::string SettingsElement::getPath() const {
            return m_strPath;
        }

        std::string SettingsElement::getValue() const {
            if(auto const& elm = getFilesMap()[getFileClassName()]) {
                return (*elm)()->read(getPath());
            }
            return "";
        }

        SettingsFile::SettingsFile(std::string const& a_strClassName, FileType a_eFileType, std::string const& a_strFilePath, int a_iFileVersion)
            : m_strClassName{ a_strClassName }
            , m_eFileType{ a_eFileType }
            , m_strFilePath{ a_strFilePath }
            , m_iFileVersion{ a_iFileVersion }
        {}

        std::string SettingsFile::getClassName() const {
            return m_strClassName;
        }

        FileType SettingsFile::getFileType() const {
            return m_eFileType;
        }

        std::string SettingsFile::getFilePath() const {
            return m_strFilePath;
        }

        int SettingsFile::getFileVersion() const {
            return m_iFileVersion;
        }

        std::string SettingsFile::read(std::string const& a_strPath) const {
            /*if(auto pInfo = SettingsFileInfo::getFileInfo(getFilePath(), getFileType())) {
                if (auto val = pInfo->tree.get_optional<std::string>(a_strPath)) {
                    return *val;
                }
            }*/
            return "";
        }

        std::map<std::string, std::map<std::string,SettingsElement::CreateMethod>>& SettingsFile::getMap() {
            static std::map<std::string, std::map<std::string,SettingsElement::CreateMethod>> map{};
            return map;
        }


        class SettingsFileManager {
            struct InfoWithMutex {
                SettingsFileInfo info{};
                std::mutex mutex{};
            };
            static std::map<std::string, InfoWithMutex> m_mapInfo;

        public:
            static SettingsFileInfo::Ptr getFileInfoAndLock(std::string const& strFilename, FileType a_eFileType) {
                auto& elm = m_mapInfo[strFilename];
                elm.mutex.lock();
                if (elm.info.strFilename.empty()) {
                    elm.info.strFilename = strFilename;
                    elm.info.eFileType = a_eFileType;
                }
                //if(elm.info.strFilecontent.empty()) {
                //std::ifstream is(elm.info.strFilename, std::ios::binary);
                //if (is.is_open()) {
                //    std::stringstream buffer;
                //    elm.info.strFilecontent.str(std::string()); // clear
                //    elm.info.strFilecontent << is.rdbuf();
                //}
                //}
                try {
                    switch(elm.info.eFileType) {
                    case FileType::XML:
                        boost::property_tree::read_xml(elm.info.strFilecontent, elm.info.tree);
                        break;
                    case FileType::JSON:
                        boost::property_tree::read_json(elm.info.strFilecontent, elm.info.tree);
                        break;
                    case FileType::INI:
                        boost::property_tree::read_ini(elm.info.strFilecontent, elm.info.tree);
                        break;
                    }
                }
                catch(...) {
                }
                return SettingsFileInfo::Ptr{ &elm.info };
            }
            static void setFileInfoAndUnlock(std::string const& strFilename) {
                auto& elm = m_mapInfo[strFilename];
                elm.info.strFilecontent.str(std::string()); // clear
                try {
                    switch(elm.info.eFileType) {
                    case FileType::XML:
                        boost::property_tree::write_xml(elm.info.strFilecontent, elm.info.tree,
                            boost::property_tree::xml_writer_settings<decltype(elm.info.tree)::key_type>(' ', 4));
                        break;
                    case FileType::JSON:
                        boost::property_tree::write_json(elm.info.strFilecontent, elm.info.tree);
                        break;
                    case FileType::INI:
                        boost::property_tree::write_ini(elm.info.strFilecontent, elm.info.tree);
                        break;
                    }
                }
                catch(...) {
                }
                std::ofstream os(elm.info.strFilename, std::ios::binary);
                if (os.is_open()) {
                    os << elm.info.strFilecontent.str();
                }
                #ifdef DEBUG_OUTPUT
                std::cout << elm.info.strFilecontent.str() << std::endl;
                #endif
                elm.mutex.unlock();
            }
        };
        std::map<std::string, SettingsFileManager::InfoWithMutex> SettingsFileManager::m_mapInfo{};

        void SettingsFileInfo::Deleter::operator()(SettingsFileInfo* a_pObj) {
            SettingsFileManager::setFileInfoAndUnlock(a_pObj->strFilename);
        }

        SettingsFileInfo::Ptr SettingsFileInfo::getFileInfo(std::unique_ptr<SettingsFile> a_pSettingsFile) {
            return SettingsFileManager::getFileInfoAndLock(a_pSettingsFile->getFilePath(), a_pSettingsFile->getFileType());
        }

        std::map<std::string, SettingsFile::CreateMethod>& getFilesMap() {
            static std::map<std::string, SettingsFile::CreateMethod> map{};
            return map;
        }
    }
}