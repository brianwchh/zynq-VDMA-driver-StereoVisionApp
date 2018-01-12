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

### application in object detection 
https://youtu.be/BIUoqIPO0g8

and 

https://youtu.be/lcXE6p9kHfU


## credit goes to many people who has shared their knowledge . 
共勉一下：勤学习，多涨薪，追房价吧（equal pay for the equal work, 希望勤劳智慧996无偿/有偿加班的中国工程师们都能像“美帝国主义”的工程师的知识一样值钱，如果一定要在这薪资前面加个修饰词，我希望是睡后）。
为社区尽一点微薄之力，帮助更多的创客。

#### 吐槽下我所了解到的中国工程师的工作权力和状态。
1. 周扒皮的半夜鸡叫进化成了打鸡血的企业文化。一样的东西，不一样的玩法，不一样的笑果。
2. 以前打电话给一个朋友，说老板还没走，不敢准点下班！
3. 老板9点以后发加班同事的照片到群里去裱羊的，有没有？
4. 9天8小时本是几个世纪工人运动流血换来的人类文明，有些地方却总是逆着文明而动。可怜之人，也必有可恨之处。


