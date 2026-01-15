set(incs include  .)
set(srcs ap_main.c)

list(APPEND incs
    ./test
    ./blur
    ./palette
    ./benchmark
    ./components
    ./components/multi_lcd
    ./utills
    ../bk7258_sdk/ap/components/multimedia/include
)

list(APPEND srcs
    ./blur/blur.c
    ./palette/palette.c
    ./test/mlcd_test.c
    ./components/dev_power.c
    ./components/multi_lcd/multi_lcd.c
    ./utills/time_labels.c
)

set(priv_req  media_service multimedia bk_display)

armino_component_register(SRCS "${srcs}" INCLUDE_DIRS "${incs}" PRIV_REQUIRES "${priv_req}")

# Copy .it.csv file to build directory
file(COPY ${CMAKE_CURRENT_LIST_DIR}/../.it.csv DESTINATION ${CMAKE_BINARY_DIR}/)

