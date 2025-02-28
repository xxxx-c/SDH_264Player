H264 Video Decoder

参考开源工程：https://github.com/jfu222/h264_video_decoder_demo
- blog: https://blog.csdn.net/jfu22/article/details/113811529

2. SDH264Player工程是相应的Windows下的H264视频播放器



## Visual Studio 版本要求
1. 开发时使用的是vs2019版本
2. 使用的是开源代码的vs2013系列解决方案


## 编译说明
1. 在Windows上，若要编译成命令行二进制可执行程序，请打开h264_video_decoder_demo_vs2013.sln，编译后会生成：
   .\h264_video_decoder_demo\build\x64\Debug\h264_video_decoder_demo_vs2013.exe

2. 在Windows上，若要编译生成视频播放器，请先打开h264_video_decoder_demo_static_vs2013.sln，编译后会生成静态链接库：
   .\h264_video_decoder_demo\build_static\x64\Debug\h264_video_decoder_demo_static_vs2013.lib
   然后再打开SDH264Player.sln，编译后会生成：
   .\h264_video_decoder_demo\SDH264Player\x64\Debug\SDH264Player.exe
   
## 任务
1. 将开源代码器替换成ffmpeg解码器，目前在ffmpeg7.1的版本上没问题
2. 替换成6.1时出现内存未分配
   

