function(set_windows_manifest)
    set(MANIFEST_FILE "${CMAKE_SOURCE_DIR}/agent/service/app.manifest")

    add_custom_command(
        TARGET wazuh-agent
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Embedding manifest in the executable..."
        COMMAND mt.exe -nologo -manifest ${MANIFEST_FILE} -outputresource:$<TARGET_FILE:wazuh-agent>; # 1
        COMMENT "Embedding manifest in ${CMAKE_CURRENT_BINARY_DIR}/wazuh-agent")
endfunction()
