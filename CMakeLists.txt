# READ ME FIRST
# This file is not to be used by cmake directly.
# This file is only to be used by idf.py build system and
# only when the project is used as ESP-IDF component

idf_component_register(
    SRCS
        "src/placeholders.cpp"
        "src/Token.cpp"
    INCLUDE_DIRS
        "src"
)