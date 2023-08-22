#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "../src/include/EmbSettings.hpp"

EMBSETTINGS_FILE(File, JSON, "@{dir}/File.xml", 1)
EMBSETTINGS_SCALAR(Scalar, int, File, "file.key", 1)

TEST_CASE("SettingsFile_static_properties") {
    SECTION("File name") {
        REQUIRE(std::string("File") == File::Name);
    }
    SECTION("File path") {
        REQUIRE(std::string("@{dir}/File.xml") == File::Path);
    }
    SECTION("File type") {
        REQUIRE(emb::settings::FileType::JSON == File::Type);
    }
    SECTION("File version") {
        REQUIRE(1 == File::Version);
    }
}

TEST_CASE("SettingElement_Scalar_static_properties") {
    SECTION("Element name") {
        REQUIRE(std::string("Scalar") == Scalar::Name);
    }
    SECTION("Element key") {
        REQUIRE(std::string("file.key") == Scalar::Key);
    }
    SECTION("Element default") {
        REQUIRE(1 == Scalar::Default);
    }
    SECTION("Element file") {
        REQUIRE(std::string("File") == Scalar::File::Name);
    }
}

TEST_CASE("SettingElement_Scalar_static_methods") {
    SECTION("Element value") {
        REQUIRE(1 == Scalar::read());
    }
    SECTION("Element default") {
        REQUIRE(Scalar::is_default());
    }
    SECTION("Element write") {
        Scalar::write(2);
        REQUIRE(2 == Scalar::read());
    }
    SECTION("Element not default") {
        REQUIRE(!Scalar::is_default());
    }
    SECTION("Element reset") {
        Scalar::reset();
        REQUIRE(Scalar::is_default());
    }
}

