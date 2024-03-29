function(build_juce_source_groups)
    get_property(all_modules GLOBAL PROPERTY _juce_module_names)
    foreach(module_name IN LISTS all_modules)
        get_target_property(path ${module_name} INTERFACE_JUCE_MODULE_PATH)
        get_target_property(header_files ${module_name} INTERFACE_JUCE_MODULE_HEADERS)
        get_target_property(source_files ${module_name} INTERFACE_JUCE_MODULE_SOURCES)
        source_group(TREE ${path} PREFIX "JUCE Modules" FILES ${header_files} ${source_files})
        set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
    endforeach()
endfunction()