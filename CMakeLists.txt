cmake_minimum_required(VERSION 3.19)
project(FinanceManager LANGUAGES CXX)

# Połączone moduły w jednym wywołaniu + dodane Charts
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Sql Charts)

qt_standard_project_setup()

qt_add_executable(FinanceManager
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    source/AddTransaction.h source/AddTransaction.cpp source/AddTransaction.ui
    source/AddCategory.h source/AddCategory.cpp source/AddCategory.ui
)

# Dodane linkowanie biblioteki Charts
target_link_libraries(FinanceManager
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Sql
        Qt6::Charts
)

install(TARGETS FinanceManager
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET FinanceManager
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})