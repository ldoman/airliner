function(generate_serialization_code)
    # Define the function arguments.
    cmake_parse_arguments(PARSED_ARGS "" "INPUT_FILE;OPS_FILE;MSGS_FILE;OUTPUT_DIR" "" ${ARGN})
    
    add_custom_target(serialization
        DEPENDS cookie 
        COMMAND ${PROJECT_SOURCE_DIR}/tools/gen_serialization/generate
         	${PARSED_ARGS_INPUT_FILE}
        	${PARSED_ARGS_OUTPUT_DIR} 
        COMMAND ${PROJECT_SOURCE_DIR}/tools/gen_serialization/merge
         	${PARSED_ARGS_OUTPUT_DIR}/cookiecutter.json
         	${PARSED_ARGS_OPS_FILE}
         	${PARSED_ARGS_MSGS_FILE}
         	${PARSED_ARGS_OUTPUT_DIR}/airliner.json
    )

    add_dependencies(ground_tools serialization)
    
endfunction(generate_serialization_code)


