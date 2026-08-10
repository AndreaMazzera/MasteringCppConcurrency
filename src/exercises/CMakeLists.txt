# Questo codice va inserito in exercises/CMakeLists.txt

# Scansiona le sottocartelle interne alla directory 'exercises'
file(GLOB current_elements CONFIGURE_DEPENDS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)

foreach(element ${current_elements})
    # Adesso ${CMAKE_CURRENT_SOURCE_DIR}/${element} punterà correttamente a:
    # exercises/00_thread_creation, exercises/01_salvadanaio_mutex, ecc.
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${element} AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${element}/main.cpp)
        
        set(TARGET_NAME ${element})
        set(TARGET_SOURCE ${element}/main.cpp)
        
        message(STATUS "🟢 Configurazione esercizio trovato: ${TARGET_NAME}")
        
        # 1. Definisce l'eseguibile
        add_executable(${TARGET_NAME} ${TARGET_SOURCE})
        
        # 2. Collega la libreria common_utils definita a livello globale
        target_link_libraries(${TARGET_NAME} PRIVATE common_utils)
        
        # 3. Configura l'installazione nella cartella bin
        install(TARGETS ${TARGET_NAME} DESTINATION bin)
        
    endif()
endforeach()