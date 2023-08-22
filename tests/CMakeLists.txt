add_executable(tests tests.cpp)
target_link_libraries(tests EmbSettings)

add_test(SettingsFile_static_properties             tests   SettingsFile_static_properties              )
add_test(SettingElement_Scalar_static_properties    tests   SettingElement_Scalar_static_properties     )
add_test(SettingElement_Scalar_static_methods       tests   SettingElement_Scalar_static_methods        )
