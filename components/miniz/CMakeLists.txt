idf_component_register(
    SRCS "miniz.c"
    INCLUDE_DIRS "."
)

# Only for miniz.c
set_source_files_properties("./miniz.c" PROPERTIES
    COMPILE_FLAGS "-Wno-type-limits -Wno-unused-function -Wno-pragmas"
)
