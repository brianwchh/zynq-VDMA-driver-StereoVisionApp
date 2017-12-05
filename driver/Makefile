#Author : Brian (Wu Cheng He)
#Email :  brianwchh@gmail.com
#github : https://github.com/brianwchh/zynq-VDMA-driver-StereoVisionApp.git

KERN_DIR=/media/brian/Files/xilinxFiles/ZynqFiles/kernel/linux
export CROSS_COMPILE=arm-xilinx-linux-gnueabi-
export ARCH=arm


all:
	make -C $(KERN_DIR) M=`pwd` modules

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

obj-m   += my_vdma.o

