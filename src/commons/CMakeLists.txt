# Crea la libreria statica includendo i sorgenti
add_library(common_utils INTERFACE)

# Dice a CMake di rendere visibile questa cartella a chiunque linki 'common_utils'
target_include_directories(common_utils INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
