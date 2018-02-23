project(meta-tokenizers)

add_library(meta-tokenizers icu_tokenizer.cpp)
target_link_libraries(meta-tokenizers meta-utf meta-util cpptoml)

install(TARGETS meta-tokenizers
        EXPORT meta-exports
        DESTINATION lib)
