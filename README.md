# zjson
## 概述
现在存在很多 Json 解析器，各有特点，各自的目标均是为使用者提供更高的解析性能，但这些解析器却往往存在如下缺陷：    
1. 容易造成运行栈攻击：因为这些解析器普遍采用递归解析方式，当 Json 数据递归层次比较高时，一个 200 KB 的 Json 文件便会使这些解析器崩溃；    
2. 网络适应性较差：需要一次性将完整 Json 数据读完后输入 Json 解析器才能进行解析，如果 Json 文件较大（比如：100 MB），则需要预先创建较大的内存；    
3. 内存占用高，易造成内存碎片：每创建一个节点对象需要多次申请内存，节点比较多时，一方面内存碎片严重，另一方面内存占用也较大，造成浪费。    

zjson 采用有限状态机方式，主要特点如下：    
1. 运行栈安全性好：流式解析，没有递归，不会造成运行栈溢出；  
2. 网络 IO 适应性更好：可以边读边解析，阻塞式/非阻塞式网络 IO 模型均适用；    
3. 内存占用低：内部采用分片内存池管理内存，内存占用更少、小内存分配更快。    

## 编译
### 编译条件
  - cmake > 2.8
  - gcc/g++ > 4.8.5
### 编译过程
1. 编译库及例子
  直接在项目目录下运行：make，则会在项目目录下生成 libzjson.a 和 libzjson.so，同时会在 example 目录下生成测试例子，在 tools 目录下生成创建多级嵌套 Json 数据的工具；  

2. 编译其它测试例子
  在 tests/ 目录有三个例子，分别是使用 zjson，cJSON 和 rapidjson 库编写的例子，在编译 cjson 例子前，需要将编译好的 cJSON 库安装至 /usr/local/lib 目录；在编译 rapidjson 前，需要将 rapidjson 的头文件所在目录 rapidjson 拷贝至 /usr/local/include/ 目录下。

### 测试
1. 进入 tools/ 目录，运行工具: `./json_create -n 100000`，其中 n 后面指定所生成 Json 数据的嵌套次数，这样在 tools/ 目录下便生成一个 Json 数据文件 json.txt；  
2. 进入 tests/cjson 目录，运行 cJSON 的测试例子：`./cjson -f ../../tools/json.txt`，此时感觉也许一切正常，这是因为 cJSON 内部有个宏定义 `#define CJSON_NESTING_LIMIT 1000` 即将嵌套次数限制在 1000 以内以避免栈溢出，可以把此值放大，比如设成 100000000，然后重新编译 cJSON 并将编译好的库安装至 /usr/local/lib/ 目录，然后再重新编译 tests/cjson 例子进行测试，看看嵌套次数达到多少时 cJSON 解析器会崩溃；  
3. 进入 tests/rapidjson 目录，运行 rapidjson 的测试例子，看看嵌套层数到多少时该程序也会崩溃。
4. 进入 test3/zjson 目录，测试采用 zjson 的例子。
