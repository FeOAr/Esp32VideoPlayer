
# 一，开发环境

## 1. 系统
Ubuntu 22.04.1 LTS

## 2. QT

![](https://imgforfeoar-1312132618.cos.ap-shanghai.myqcloud.com/markdown/202302062001654.png)

# 二，上位机相关
1. 用 QT Creator 添加“Esp32VideoPlayer/Player/videoforesp32/”目录下的“. Pro”文件，或是直接下载 release 版本，直接运行下图文件，可能会有 QT 框架的安装依赖。 ![](https://imgforfeoar-1312132618.cos.ap-shanghai.myqcloud.com/markdown/202302062011149.png)
2. 上位机软件使用方法
先选择一个视频文件，根据视频的特点选择要不要反色以获得更好的显示效果。然后点击处理视频，会在当前目录下生成一个 cache 文件夹，存放临时文件。然后随便设置一个端口，小于 65535 就行，点击开启监听。esp32 连上后会有提示。收到“hello world”后就可以发送了。
P.S. 由于软件不完善，不建议软件开启一次，处理多次视频，但可以发送多次。并且只要不删 cache，下次开启就不用再次处理相同视频了。会提示检测到。
![](https://imgforfeoar-1312132618.cos.ap-shanghai.myqcloud.com/markdown/202302062027414.png)

# 三，下位机相关
可用 Arduino IDE 2.0 打开“. ino”文件，编译上传。**由于下位机的代码也是在 ubuntu 下开发**，可能会有编码问题。

注意：上传前，修改端口，IP，你的 wifi 名字和密码。
![](https://imgforfeoar-1312132618.cos.ap-shanghai.myqcloud.com/markdown/202302062032989.png)

# 四，Q&A
1. 板子连上 Ubuntu 后认不到
答：如果是 Ubuntu22，有个给残疾人交互的东西占用了端口，CSDN 搜一下，卸载掉就好。如果是其他版本，可能是没有装好驱动。
2. 板子连好了但是 Arduino IDE 上传不上去，显示类似“ttyUSB0 does not exist”类似字眼。
答：可能是没有给端口权限，在 Ubuntu 下一切接文件。可以直接 `sudo chmod +777 /dev/ttyUSB0` 等类似操作。
3. 上传时 connect..... 一直连不上。
答：可能自动上传失效，需要你按住 boot 键，在按一下 reset 键。然后差不多就可以连上了。

