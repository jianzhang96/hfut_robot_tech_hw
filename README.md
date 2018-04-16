## 合肥工业大学《机器人技术》作业代码
**每个目录下写了Makefile，Linux系统下可以直接用make命令编译**<br/>

### 第一次：Parse
从一条字符串里面先分割出每一条see和hear的信息。hear解析相对简单，数据的格式是固定的；see信息要解析出没一条ObjectInfo，对与每一条ObjectInfo，再分类别解析。分割字符串调用标准库<cstring>里面的strtok()函数。因为没太懂题目给标记的意思，代码对发来的信息对象进行了一个判断strinOjects()。<br/>
字符串从文件in.txt读取，输出到终端和文件。<br/>

### 第二次：Gemo
Gemeotry.h里面声明了四个类，一些定义也直接写在里面了，Gemeotry.cpp里面主要是直线和直线、圆、矩形求交点的函数。main()里面示例调用。求解的方法就是用几何解析法代公式。<br/>
这份代码基本是从agent2d-3.1的rcsc里面移过来的。

### 第三次：locate.py
用python写的，求解的方法见教程106页。
