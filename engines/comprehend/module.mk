MODULE := engines/comprehend

MODULE_OBJS :=				\
	detection.o			\
	comprehend.o			\
	comprehend_transylvania.o	\
	game_data.o			\
	image_manager.o			\
	image_file.o			\
	renderer.o			\
	console.o			\
	parser.o

ifeq ($(ENABLE_COMPREHEND), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

include $(srcdir)/rules.mk
