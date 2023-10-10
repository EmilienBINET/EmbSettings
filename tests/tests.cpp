#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "../src/include/EmbSettings.hpp"

EMBSETTINGS_FILE(File, JSON, "@{dir}/File.xml", 1, nullptr)
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
        REQUIRE_FALSE(Scalar::is_default());
    }
    SECTION("Element reset") {
        Scalar::reset();
        REQUIRE(Scalar::is_default());
    }
}

TEST_CASE("SettingElement_transactions") {
    SECTION("Transaction abortion") {
        Scalar::write(1234);
        REQUIRE(1234 == Scalar::read());
        File::begin();
        Scalar::write(5678);
        //REQUIRE(1234 == Scalar::read());
        File::abort();
        REQUIRE(1234 == Scalar::read());
    }
    SECTION("Transaction commit") {
        Scalar::write(9876);
        REQUIRE(9876 == Scalar::read());
        File::begin();
        Scalar::write(5678);
        //REQUIRE(9876 == Scalar::read());
        File::commit();
        REQUIRE(5678 == Scalar::read());
    }
}
