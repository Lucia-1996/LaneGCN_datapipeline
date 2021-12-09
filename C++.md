# C++

## : 和 ::

类名后边的:用来定义类的继承，具体用法 class 派生类名:继承方式 基类名{派生类的成员}

双冒号

1. 表示“域操作符”：声明类A，类A里声明了成员函数`void f()`，但没有在类的声明里给出f的定义，那么在类外定义f时，要写成 `void::f()`，表示这个函数`f()`是类A的成员函数。

2. "作用域分解运算符"

## 类的构造函数和析构函数

类的构造函数每次在创建类的新对象的时候执行，函数名称与类的名称相同，不返回任何类型，也不返回void。构造函数可以为某些成员变量设置初始值。

默认的构造函数没有任何参数，如果需要也可以。

类的**析构函数**是类的一种特殊的成员函数，它会在每次删除所创建的对象时执行。函数名称与类的名称相同，只是在前面加了一个波浪号前缀，不返回任何值，不带有任何参数。析构函数有助于跳出程序前释放资源。

## .和->

当定义类对象是一般对象时候需要用"."来指向类中的成员，当定义类对象是指针对象的时候，需要用"->"指向类中的成员

定义：`A *p `   则：`p->play()`    左边是结构指针

定义：`A p   ` 则：`p.play()`      左边是结构变量

## 模板<>

模板（template）

举例：swap函数

```C++
void swap(int&a , int& b) {
    int temp = a;
    a =  b;
    b = temp;
}
```

但是如果支持long、string，代码和上述代码差不多，只是类型不同，这时就是我们定义swap的函数模板，可以复用不同类型的swap函数代码，函数模板的声明形式如下：

```C++
//method.h
template<typename T> void swap(T& t1, T& t2);

#include "method.cpp"

//method.cpp
template<typename  T> void swap(T& t1, T& t2) {
    T tmpT;
    tmpT = t1;
    t1 = t2;
    t2 = tmpT;
}
```

使用模板：

```C++
//main.cpp
#include <stdio.h>
#include "method.h"
int main() {
    //模板方法 
    int num1 = 1, num2 = 2;
    swap<int>(num1, num2);
    printf("num1:%d, num2:%d\n", num1, num2);  
    return 0;
}
```

## `Protocol Buffer Basics:C++`

`protocal buffers`是针对序列化和获取结构化数据而提出的解决方案。使用`protocal buffers`需要写一个`.proto`说明，用于描述所希望存储的数据结构。利用 `.proto` 文件，protocol buffer 编译器可以创建一个类，用于实现自动化编码和解码高效的二进制格式的 protocol buffer 数据。产生的类提供了构造 `protocol buffer` 的字段的 getters 和 setters，并且作为一个单元，关注读写 `protocol buffer` 的细节。重要的是，`protocol buffer` 格式支持扩展格式，代码仍然可以读取以旧格式编码的数据。

### 定义协议格式

`.proto` 文件中的定义很简单：为你所需要序列化的数据结构添加一个消息（message），然后为消息中的每一个字段指定一个名字和类型。这里是定义你消息的 `.proto` 文件，`addressbook.proto`。

```C++
package tutorial;//.proto 文件以一个 package 声明开始，这可以避免不同项目的命名冲突。在 C++，你生成的类会被置于与 package 名字一样的命名空间。
//定义消息：消息只是一个包含一系列类型字段的集合。
//大多标准简单数据类型是可以作为字段类型的，包括 bool、int32、float、double 和 string。你也可以通过使用其他消息类型作为字段类型，将更多的数据结构添加到你的消息中,比如，Person消息中包含了PhoneNumber消息
//每一个元素上的 “=1”、"=2" 标记确定了用于二进制编码的唯一"标签"（tag）。标签数字 1-15 的编码比更大的数字少需要一个字节，因此作为一种优化，你可以将这些标签用于经常使用或 repeated 元素，剩下 16 以及更高的标签用于非经常使用或 optional 元素。每一个 repeated 字段的元素需要重新编码标签数字，因此 repeated 字段对于这优化是一个特别好的候选者。
//每一个字段必须使用下面的修饰符加以标注：1.required：必须提供字段的值，否则消息会被认为是 "未初始化的"（uninitialized）。2.optional：字段可能会被设置，也可能不会。如果一个 optional 字段没被设置，它将使用默认值。3.repeated：字段可以重复任意次数（包括 0）。


message Person {
  required string name = 1;
  required int32 id = 2;
  optional string email = 3;

  enum PhoneType {
    MOBILE = 0;
    HOME = 1;
    WORK = 2;
  }

  message PhoneNumber {
    required string number = 1;
    optional PhoneType type = 2 [default = HOME];
  }

  repeated PhoneNumber phone = 4;
}

message AddressBook {
  repeated Person person = 1;
}
```

### 编译`Protocal Buffers`

在 `.proto` 上运行 protocol buffer 编译器 `protoc`

```C++
protoc -I=$SRC_DIR --cpp_out=$DST_DIR $SRC_DIR/addressbook.proto
```

在你指定的目标文件夹，将生成以下的文件：`addressbook.pb.h`，声明你生成类的头文件。`addressbook.pb.cc`，包含你的类的实现。

让我们看看生成的一些代码，了解一下编译器为你创建了什么类和函数。如果你查看 `tutorial.pb.h`，你可以看到有一个在 `tutorial.proto` 中指定所有消息的类。关注 `Person` 类，可以看到编译器为每个字段生成了读写函数（accessors）。例如，对于 `name`、`id`、`email` 和 `phone` 字段，有下面这些方法：

```C++
// name
inline bool has_name() const;
inline void clear_name();
inline const ::std::string& name() const;
inline void set_name(const ::std::string& value);
inline void set_name(const char* value);
inline ::std::string* mutable_name();

// id
inline bool has_id() const;
inline void clear_id();
inline int32_t id() const;
inline void set_id(int32_t value);

// email
inline bool has_email() const;
inline void clear_email();
inline const ::std::string& email() const;
inline void set_email(const ::std::string& value);
inline void set_email(const char* value);
inline ::std::string* mutable_email();

// phone
inline int phone_size() const;
inline void clear_phone();
inline const ::google::protobuf::RepeatedPtrField< ::tutorial::Person_PhoneNumber >& phone() const;
inline ::google::protobuf::RepeatedPtrField< ::tutorial::Person_PhoneNumber >* mutable_phone();
inline const ::tutorial::Person_PhoneNumber& phone(int index) const;
inline ::tutorial::Person_PhoneNumber* mutable_phone(int index);
inline ::tutorial::Person_PhoneNumber* add_phone();
```

getters 的名字与字段的小写名字完全一样，并且 setter 方法以 set_ 开头。同时每个单一（singular）（required 或 optional）字段都有 `has_` 方法，该方法在字段被设置了值的情况下返回 true。最后，所有字段都有一个 `clear_` 方法，用以清除字段到空（empty）状态。

*11.30-12.2 编写getActorFeats.cpp代码*

### C++ 读`.csv`文件

两种方式，一种可以直接读写，另一种是调用第三方库。

直接读写：

```c++
#include <iostream>  
#include <string>  
#include <vector>  
#include <fstream>  
#include <sstream>  
  
using namespace std;  
  
  
int main()  
{  
    // 写文件  
    ofstream outFile;  
    outFile.open("data.csv", ios::out); // 打开模式可省略  
    outFile << "name" << ',' << "age" << ',' << "hobby" << endl;  
    outFile << "Mike" << ',' << 18 << ',' << "paiting" << endl;  
    outFile << "Tom" << ',' << 25 << ',' << "football" << endl;  
    outFile << "Jack" << ',' << 21 << ',' << "music" << endl;  
    outFile.close();  
  
    // 读文件  
    ifstream inFile("data.csv", ios::in);  
    string lineStr;  
    vector<vector<string>> strArray;  
    while (getline(inFile, lineStr))  
    {  
        // 打印整行字符串  
        cout << lineStr << endl;  
        // 存成二维表结构  
        stringstream ss(lineStr);  
        string str;  
        vector<string> lineArray;  
        // 按照逗号分隔  
        while (getline(ss, str, ','))  
            lineArray.push_back(str);  
        strArray.push_back(lineArray);  
    }  
      
    getchar();  
    return 0;  
}
```

使用库：

https://github.com/d99kris/rapidcsv

### `Eigen`的安装与使用

前言：在`VScode`上新建工程，`VScode`是以文件夹的形式管理工程的，因此首先新建一个文件夹，用vscode打开文件夹，新建cpp文件

下载`Eigen`，解压，将

`ctrl+shift+p`调出命令行，选择`C/C++: Edit Configurations(UI)`，点击`c_cpp_properties.json`打开配置文件。 在`IncludePath `内加上需要的头文件路径即可，

`/usr/include/` `/usr/local/include`

所以这两个基本要加上的，如果你不知道你安装的库的头文件在哪，但是知道关键的头文件名称，则可以用 `locate` 命令来查找：

`locate  ros.h| grep include`

这个命令的意思是查找所有ros.h的位置，并且找出路径中带有 include 字段的路径。最终的`c_cpp_properties.json`配置如下：

```
{
  "configurations": [
    {
      "name": "Linux",
      "includePath": [
        "${workspaceFolder}/**",
        "/usr/include/",
        "/usr/local/include/",
        "/usr/include/eigen3/"
      ],
      "defines": [],
      "compilerPath": "/usr/bin/gcc",
      "cStandard": "c11",
      "cppStandard": "c++11",
      "intelliSenseMode": "clang-x64"
    }
  ],
  "version": 4
}
```



将`Eigen` 复制到默认include路径下

```
cp Eigen/ /usr/include/ -r
```

使用`Eigen::matrix`:

```C++
   Eigen::Matrix<double, 20, 3, Eigen::RowMajor> feat;//20*3的矩阵
   for (int j = beg; j < 20; ++j)
        {
            feat(j, 0) = tmp(j - beg, 0);//给矩阵赋值
            feat(j, 1) = tmp(j - beg, 1);
            feat(j, 2) = 1.0;
        }
```

### map 的使用

map 是自动按照key值排序的，不能保证原来的插入顺序，不能对map使用sort函数。

```
std::map<std::string, std::vector<int>> actor_idcs;//map声明
```

 **<u>map 插入数据的三种方式：</u>**

(1) insert函数插入pair数据。

```C++
std::map < int , std::string > mapPerson;
mapPerson.insert(pair < int,string > (1,"Jim"));
```

(2) insert函数插入value_type数据。

```C++
mapPerson.insert(std::map < int, std::string > ::value_type (2, "Tom"));
```

(3) 数组方式插入数据。

```C++
mapPerson[3] = "Jerry";
```

<u>**map 的遍历：**</u>

（1）前向迭代器：

```C++
std::map < int ,std::string > ::iterator it;
    std::map < int ,std::string > ::iterator itEnd;
    it = mapPerson.begin();
    itEnd = mapPerson.end();
    while (it != itEnd) {
	cout<< it->first <<' '<< it->second <<endl;  
	it++;
}
```

(2)反向迭代器：

```C++
std::map < int, string > ::reverse_iterator iter;  
for(iter = mapPerson.rbegin(); iter != mapPerson.rend(); iter++) 
	cout<<iter->first<<"  "<<iter->second<<endl;  
```

(3)数组形式：

```C++
mapPerson.insert(std::map<int, std::string>::value_type (1, "Tom"));
mapPerson[2] = "Jim";
mapPerson[3] = "Jerry";

int nSize = mapPerson.size();
for(int n = 1; n <= nSize; n++)
	qDebug()<<QString::fromStdString(mapPerson[n]);
```

(优先使用前向迭代器，慎用数组形式，角标问题注意)。

**<u>map 元素删除</u>**：

```C++
iterator erase（iterator it)	;//通过一个条目对象删除
iterator erase（iterator first，iterator last）；	//删除一个范围
size_type erase(const Key&key);	//通过关键字删除
clear()；//就相当于enumMap.erase(enumMap.begin(),enumMap.end());
```

**<u>map 清空所有键值对：</u>**

```
map.clear();
```

使用set或者vector作为map的value，插入时：

```C++
   map<pair<int, int>, set<int>> tree_map;
    tree_map[{7, 8}].insert(8);
    tree_map[{5, 6}].insert(6);
    tree_map[{5, 6}].insert(7);
    //(7,8):8
    //(5,6):6,7
    map<string, vector<int>> actor_idcs;
    actor_idcs["01"].push_back(1);
    actor_idcs["02"].push_back(1);
    actor_idcs["01"].push_back(2);
    //"01":[1,2]
    //"02":[1]
```

### vector 的使用

vector 有点变长数组的意思。

**<u>数组转换为vector：</u>**

```C++
int arr[] = {1,2,3,4,5};
vector<int> varr(arr, arr+5);
```

<u>**判断 vector 是否为空：**</u>

```C++
!v[n].empty()
```

**<u>基本操作：</u>**

头文件：`#include<vector>`

创建 vector 对象：`vector<int> vec`

尾部插入数字：`vec.push_back(a)`

使用下标访问元素：`vec[0]`

使用迭代器访问元素：

```C++
vector<int>::iterator it;
for(it=vec.begin();it!=vec.end();it++)
    cout<<*it<<endl;
```

插入元素：`vec.insert(vec.begin()+i,a)`; 在第i+1个元素前面插入a;

删除元素：`vec.erase(vec.begin()+2)` ; 删除第3个元素；`vec.erase(vec.begin()+i,vec.end()+j)`; 删除区间[ i,j-1] 区间从0开始

向量大小: `vec.size()`;

清空: `vec.clear()`;

vector 的元素不仅仅可以是 int,double,string 还可以是结构体，但是要注意：结构体要定义为全局的，否则会出错。

使用reverse将元素翻转：需要头文件 #include<algorithm>；`reverse(vec.begin(),vec.end());`将元素翻转，即逆序排列！

使用 sort 排序：需要头文件 #include<algorithm>；`sort(vec.begin(),vec.end());`(默认是按升序排列,即从小到大).

**<u>二维数组的使用：</u>**

```C++
#include "stdafx.h"  
#include <cv.h>  
#include <vector>   
#include <iostream>   
using namespace std;  
int main()  
{  
    using namespace std;  
    int out[3][2] = { 1, 2,   
             3, 4,  
            5, 6 };  
    vector <int*> v1;  
    v1.push_back(out[0]);  
    v1.push_back(out[1]);  
    v1.push_back(out[2]);  
    cout << v1[0][0] << endl;//1  
    cout << v1[0][1] << endl;//2  
    cout << v1[1][0] << endl;//3  
    cout << v1[1][1] << endl;//4  
    cout << v1[2][0] << endl;//5  
    cout << v1[2][1] << endl;//6  
    return 0;  
}  
```

unique()函数可以给vector去重，但是之前必须排序，将重复的值放在一起，否则不行；

### C++ 不用科学计数法输出以及保留小数位数

```C++
void print_gt_pred(Eigen::Matrix<double, 30, 2, Eigen::RowMajor> gt_pred){
    std::cout.setf(std::ios::fixed,std::ios::floatfield);//改为十进制输出结果
    for (int i = 0;i<30;i++){
        for(int j = 0;j<2;j++){
            std::cout<<std::setprecision(5) << double(gt_pred(i,j))<<",";//显示小数点后5位
        }
        std::cout<<std::endl;
    }
    std::cout<<"==============================="<<std::endl;
}
```

