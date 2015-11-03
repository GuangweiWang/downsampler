
PROJ_NAME := downsampler


all: $(PROJ_NAME)

$(PROJ_NAME):
	gcc ../src/main.c ../src/downsamplefuncs.c -o $(PROJ_NAME)

install:
	cp $(PROJ_NAME) ~/WorkSpace/bin

uninstall:
	rm -rf ~/WorkSpace/bin/$(PROJ_NAME)

demo:
	./downsampler CAP_720P_RCOFF.yuv 1280 720 CAP_RCOff_4_general.yuv CAP_RCOff_4_specific.yuv 320 180
	./downsampler CAP_RCOFF_480x270.yuv 480 270 CAP_RCOff_3_general.yuv CAP_RCOff_3_specific.yuv 160 90

clean:
	rm -rf $(PROJ_NAME)
	rm -rf out_*.yuv
	rm -rf *_general.yuv *_specific.yuv
	rm -rf log.txt

