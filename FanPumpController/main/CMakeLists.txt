#Edit following two lines to set component requirements (see docs)
idf_component_register(
SRCS 

	"main.c"
	"timer.c"
	"rs485.c"
	"i2c.c"
	"display.c"
	"touch.c"
	"crc.c"
	"adc.c"
	"ntc.c"
	"menu.c"
	"struckt.c"
#	"u8g2/u8g2_esp32_hal.c"
#	"u8g2/u8g2_buffer.c"
#	"u8g2/u8g2_font.c"
#	"u8g2/u8g2_setup.c"
#	"u8g2/u8g8_display.c"
	
	INCLUDE_DIRS "."
	
)

#This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

#FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

#idf_component_register(SRCS 