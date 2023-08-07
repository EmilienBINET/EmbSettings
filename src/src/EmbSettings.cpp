#include "../include/EmbSettings.hpp"
#include <string>
#define BOOST_BIND_GLOBAL_PLACEHOLDERS // Avoid warning
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/algorithm/string/replace.hpp>
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

        map<string, string>& jockers() {
            static map<string, string> jockers{};
            return jockers;
        }

        void setJocker(std::string const& aJocker, std::string const& aValue) {
            jockers()[aJocker] = aValue;
        }

        void parseJockers(std::string& a_rstrFilePath) {
            for (auto const& elm : jockers()) {
                boost::replace_all(a_rstrFilePath, "@{" + elm.first + "}", elm.second);
            }
        }

        std::map<std::string, std::pair<std::function<void(void)>, std::function<void(void)>>>& SettingsElement::getLinks() {
            static std::map<std::string, std::pair<std::function<void(void)>, std::function<void(void)>>> links{};
            return links;
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

        std::string SettingsElement::read() const {
            return read<std::string>("");
        }

        void SettingsElement::write(std::string const& a_strNewValue) const {
            write<std::string>(a_strNewValue);
        }

        void SettingsElement::read_linked() const {
            get<0>(SettingsElement::getLinks()[getPath()])();
        }

        void SettingsElement::write_linked() const {
            get<1>(SettingsElement::getLinks()[getPath()])();
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

        std::map<std::string, std::map<std::string, SettingsElement::CreateMethod>>& SettingsFile::getMap() {
            static std::map<std::string, std::map<std::string, SettingsElement::CreateMethod>> map{};
            return map;
        }

        bool SettingsFile::register_settings(char const* a_szFile, char const* a_szPath, SettingsElement::CreateMethod a_pCreateMethod) {
            #ifdef DEBUG_REGISTER
            std::cout << "Registering Setting: " << a_szPath << " in " << a_szFile << std::endl;
            #endif
            getMap()[a_szFile][a_szPath] = a_pCreateMethod;
            return true;
        }

        std::map<std::string, emb::settings::SettingsElement::CreateMethod>& SettingsFile::getElementsMap(std::string const& a_strFileClass) {
            return getMap()[a_strFileClass];
        }

        class SettingsFileManager {
            struct InfoWithMutex {
                SettingsFileInfo info{};
                std::mutex mutex{};
                std::string strFullFileName{};
                FileType eFileType{};
                std::stringstream strFilecontent{};
            };
            static std::map<std::string, InfoWithMutex> m_mapInfo;

        public:
            static SettingsFileInfo::Ptr getFileInfoAndLock(std::string const& strFilename, FileType a_eFileType) {
                auto& elm = m_mapInfo[strFilename];
                elm.mutex.lock();
                if (elm.info.strFilename.empty()) {
                    elm.strFullFileName = strFilename;
                    parseJockers(elm.strFullFileName);
                    elm.info.strFilename = strFilename;
                    elm.eFileType = a_eFileType;
                    std::ifstream is(elm.strFullFileName, std::ios::binary);
                    if (is.is_open()) {
                        std::stringstream buffer;
                        elm.strFilecontent.str(std::string()); // clear
                        elm.strFilecontent << is.rdbuf();
                    }
                    try {
                        switch (elm.eFileType) {
                        case FileType::XML:
                            boost::property_tree::read_xml(elm.strFilecontent, elm.info.tree);
                            break;
                        case FileType::JSON:
                            boost::property_tree::read_json(elm.strFilecontent, elm.info.tree);
                            break;
                        case FileType::INI:
                            boost::property_tree::read_ini(elm.strFilecontent, elm.info.tree);
                            break;
                        }
                    }
                    catch (...) {
                        elm.info.tree = decltype(elm.info.tree)();
                    }
                }
                return SettingsFileInfo::Ptr{ &elm.info };
            }
            static void setFileInfoAndUnlock(std::string const& strFilename) {
                auto& elm = m_mapInfo[strFilename];
                try {
                    std::stringstream strFilecontent{};
                    switch (elm.eFileType) {
                    case FileType::XML:
                        boost::property_tree::write_xml(strFilecontent, elm.info.tree/*,
                            boost::property_tree::xml_writer_settings<decltype(elm.info.tree)::key_type>(' ', 4)*/);
                        break;
                    case FileType::JSON:
                        boost::property_tree::write_json(strFilecontent, elm.info.tree);
                        break;
                    case FileType::INI:
                        boost::property_tree::write_ini(strFilecontent, elm.info.tree);
                        break;
                    }
                    if (elm.strFilecontent.str() != strFilecontent.str()) {
                        std::ofstream os(elm.strFullFileName, std::ios::binary);
                        if (os.is_open()) {
                            os << strFilecontent.str();
                        }
                    }
                    elm.strFilecontent.str(strFilecontent.str());
                }
                catch (...) {
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