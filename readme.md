in this repository I share a zynq-vdma driver I used in stereo vision ,and an demo app that utilize this driver . 
I used to have trouble with this driver,and had spend sometime to figure it out。it might not be a standard way, but it works well in this case.
I hope this can help some FPGA guys . if there is something wrong, pls kindly correct me .

note that the main job this app does is as follows : 
1. read rectified images , (this FPGA bit stream file is used to test KITTI dataset, so the rectify module is commended out in verilog HDL project)
2. load it to DDR memory .
3. triger the stereo IP core in FPGA side to generate depth image, depending whether you press 'c' or 'g', it shows color or gray depth image respectively

![alt text](https://raw.githubusercontent.com/brianwchh/zynq-VDMA-driver-StereoVisionApp/master/ipblock/ipblock.png)


## demo videos : 

### realtime FPGA stereo vision demo :

https://youtu.be/se1tIy5UWpE  

or 

http://v.youku.com/v_show/id_XMzAwNTcxNDA0OA==.html?spm=a2hzp.8253869.0.0

### precision test : 

https://youtu.be/iO0MZLmZf58  

or  

http://v.youku.com/v_show/id_XMzE3NjU0NTA4NA==.html?spm=a2hzp.8253869.0.0


## QQ group : 150241908

## credit goes to many people who has shared their knowledge . 
共勉一下：勤学习，多涨薪，追房价（按现阶段的物价，动动指头，先赚他个4-6万/月吧，哈哈^_^，开心就好）。为社区尽一点微薄之力，帮助更多的创客。

