# zjson
## 概述
zjson 是一个流式 Json 解析库，每次可以给解析器输入部分 Json 数据，边输入边解析，而大多数解析器需要一次性将完整的 Json 数据输入才能解析，流式解析方式更加灵活方便，可以从网络 IO 中边读边解析。另外，zjson 流式解析器采用有限状态机方式，而大多数解析器采用递归方式，zjson 不会因 Json 数据嵌套层次太深而导致栈溢出，采用递归方式的解析器却存在这种问题，可能会导致栈溢出攻击。

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
