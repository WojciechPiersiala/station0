idf_component_register(SRCS "main.c" 
                            "../src/wifi.c" 
                            "../src/tcp_client.c" 
                            "../src/mic.c" 
                            "../src/display.c"
                            "../src/display_text.c"
                            "../src/battery.c"
                    INCLUDE_DIRS "." "../inc" 
                    "C:/Espressif/frameworks/esp-idf-v5.3.1/examples/peripherals/i2s")
