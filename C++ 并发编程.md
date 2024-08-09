## 向可调用对象进行参数传递
### 参数传递方式
#### 按值复制
向一个线程对象`std::thread t`传递对象只需将参数作为其构造函数的参数即可。 ^9eb1c7
```cpp
void f(int,const int& a);

int n=1;
std::thread t{f,0,n};
```
但是这里的参数传递实际上都是通过`复制`传递的，即使在函数的定义中是引用传递。
```cpp
void f1(int,  int& a)
{
	std::cout << "子线程开始" << std::endl;
	std::cout << "参数a:" << a << " 参数a的地址:" << &a << std::endl;
	std::cout << "子线程结束" << std::endl;
}

int main()
{
	cout << "主线程开始" << endl;
	int n = 1;
	cout << "参数n:" << n << " 参数n的地址:" << &n << endl;
	thread t{ f1,3,n };
	t.join();
	cout << "主线程结束" << endl;

	return 0;
}
```
执行结果如下：
```
主线程开始
参数n:1 参数n的地址:0000007B65EFF584
子线程开始
参数a:1 参数a的地址:00000180FAC83E30
子线程结束
主线程结束
```
如结果所示，即使在函数定义中第二个参数为常量引用传递，但实际上参数传递到子线程的方式依然是按值复制。
而如果将函数中的`const` 去掉，改为普通的引用传递，则无法通过编译，会出现如下的错误。
```
严重性	代码	说明	项目	文件	行	禁止显示状态
错误	C2672	“invoke”: 未找到匹配的重载函数	multiThreadTest	C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.33.31629\include\thread	55	
```
这是因为`std::thread`的构造函数所需要的参数是`可复制或可移动`类型的，而一个非const引用是不可复制的类型，线程在创建时会将参数的副本通过复制的方式传递到子进程，所以非const引用无法正常传递到子进程。
那如果就是想要传递引用呢？这时可以使用`std::ref`或`std::cref`包装一下参数。
> 何为`std::ref`和`std::cref`？
> `std:ref`是C++ 标准库中的一个函数模板，用于包装一个左值引用，使其可以被存储在需要`可调用对象`的上下文中，同时保持其语义。
> 简单地说，但你需要将一个左值`n`传递到一个需要可调用对象的函数或容器时，同时想要保持对该左值的引用而不是按值传递，就可以通过`std::ref(n)`(如果左值是常量`const`则是`std::cref(n)`)传递。
> 可以理解为`std::ref` 将你提供的左值进行包装，并返回一个“引用”，当然实际上并不是真的引用，而是一个包装类`std::reference_wrapper`(`std::cref`就是`std::reference_wrapper<const T>`)，这个类就是包装引用对象类模板，将对象包装，可以隐式转换为被包装对象的引用。
#### 转化为右值表达式
还有一点，`std::thread`内部在传递参数时其实会将参数转换为`右值表达式`。
> 这是因为当传递的参数是`只支持移动`的类型，也就是说所传递的参数的复制构造函数被删除或不可用，但移动构造函数可用，`std::thread`就不能简单的通过复制传递参数的副本。

如下代码，向线程对象传递的参数为右值，没有使用`std::ref`进行包装
```cpp
//参数是右值传递
struct can_only_move
{
	can_only_move() { std::puts("can_only_move的默认构造函数"); }
	//删除复制构造函数
	can_only_move(const can_only_move&) = delete;
	//移动构造函数
	can_only_move(can_only_move&&)noexcept { std::puts("can_only_move的移动构造函数"); }
};
//can_only_move的测试用函数
void f2(can_only_move){}

int main()
{
	can_only_move obj;
	thread t{ f2,move(obj) };
	t.join();
	
	return 0;
}
```
以上代码的执行结果如下：
```
can_only_move的默认构造函数
can_only_move的移动构造函数
can_only_move的移动构造函数
```
可以看到，移动构造函数调用了两次，这是因为没有通过`std::ref`传递引用，所以传递的是副本。
于是第一次移动构造发生在`move(obj)`，这时是在初始化要传递的参数的副本
第二次发生在调用函数`f2`时向函数内传递参数，由于传递的是右值，而`f2`是按值接受参数，编译器会生成一个临时的`can_only_move`对象并调用其移动构造函数用于向`f2`传参。
### 成员函数指针
由于成员函数指针也是可调用对象，所以也可以传递给`std::thread`的构造函数，使线程对象关联成员函数。
```cpp
//成员函数指针作为参数传递
struct Test1
{
	void task_run(const int& a)const
	{
		std::cout << "Test1的成员函数task_run被调用，传入参数a:" << a << std::endl;
		std::cout << "参数a的地址" << &a << std::endl;
	}
};

int main()
{
	//成员函数指针作为参数传递
	Test1 test1;
	int a = 10;
	cout << "a的地址:" << &a << endl;
	thread t1{ &Test1::task_run,&test1,a };
	t1.join();
}
```
在上面的线程对象t1的构造函数中，传入了成员函数指针`&Test1::task_run`、成员函数的对象`&test1`、该函数需要的参数`a`。
其中成员函数指针的格式为`&类名::非静态成员`，而且成员函数指针要与其对应的对象一起使用。
上面的代码执行结果如下：
```
a的地址:0000006C7A6FF954
Test1的成员函数task_run被调用，传入参数a:10
参数a的地址000001984FD6F2F0
```
### 成员函数指针可以配合`std::bind`使用
上面的代码中`struct Test1`定义不变，main()函数内部更改如下：
```cpp
	//成员函数指针作为参数传递
	Test1 test1;
	int a = 10;
	cout << "a的地址:" << &a << endl;
	thread t1{ &Test1::task_run,&test1,a };
	t1.join();
	//通过bind调用
	thread t2{bind(&Test1::task_run,&test1,ref(a)) };
	t2.join();
```
> `std::bind`可以将可调用对象与其参数进行绑定生成一个新的可调用对象，该对象可以直接进行调用或者传递到需要可调用对象的地方。
> 如果要使用`std::bind`不要忘记`#include<functional>`。

注意，`std::bind`中传递参数的方式与`std::thread`的构造函数类似，默认是按值传递，所以如果想要传递引用时需要使用`std::ref`或`std::cref`。
更改后的代码运行结果如下：
```
a的地址:000000CBB6AFF8E4
Test1的成员函数task_run被调用，传入参数a:10
参数a的地址00000240C91ED930
Test1的成员函数task_run被调用，传入参数a:10
参数a的地址000000CBB6AFF8E4
```
### `std::thread`构造参数生存期问题
先上代码：
```cpp
void f(const std::string&);
void test(){
    char buffer[1024]{};
    //todo.. code
    std::thread t{ f,buffer };
    t.detach();
}
```
上面代码中有一个参数为`std::string`的函数`f`，在`test()`中构造了一个`char`数组，一个线程对象`t`关联了函数`f`并将`char` 数组`buffer`作为参数传入，最后使用`detach()`分离的方式启用线程。
这里的问题出现在线程对象`t`构造的参数传递。
函数`f`需要的参数为`std::string` 类型，但这里传入的是`char` 数组，这里会发生`衰减(decay)`。
> `decay`是指对象从一个类型（例如引用或数组）**隐式**地转换为另一个类型（例如指针或指向单个元素的指针）时。这种转换通常发生在函数参数传递、函数返回值以及模板参数推导等上下文中。
> 具体地说：
> 1. **数组到指针的转换**：当你传递一个数组给函数时，数组名会隐式地转换为指向其第一个元素的指针。这并不涉及复制数组的内容，只是传递了一个指向数组的指针。
>    
>2. **函数到函数指针的转换**：如果你传递一个函数给期望函数指针的函数，函数名会隐式地转换为指向该函数的指针。
>  
>3. **顶层const和volatile的丢失**：当你传递一个顶层const或volatile的对象给函数时，这些属性可能会在传递过程中被丢弃（除非函数参数也是const或volatile的）。

在这里`buffer`被隐式地转换为了`char *`指向`buffer`首元素，这个`char *`被保存进了新的线程，在调用`f`时会通过`std::string`的转换构造函数创建一个临时的`std::string`对象用于函数调用。
问题在于，最后调用线程对象的方式是`detach`，也就是说在在子线程执行的时候，主线程可能会先结束并释放资源，而这时`buffer`衰减成的指针就会**悬空**，这就是问题所在。
想要解决可以：
1.**通过`join()`启用线程而非`detach()`**
2.**将`buffer`转换为`std::string`，`std::string(buffer)`**
## `std::this_thread`
该命名空间用于管理当前线程，常用函数有：
**`get_id`**：用于获取当前线程的id，该id在系统范围内是唯一的。
```cpp
	//获取主线程和子线程的id
	cout << "主线程id:" << this_thread::get_id() << endl;
	thread t{ [] {std::cout << "子线程id:" << std::this_thread::get_id() << endl; } };
	t.join();
```
执行结果：
```
主线程id:17696
子线程id:24440
```
**`sleep_for`**： 使当前线程阻塞指定的时间长度。该函数接受一个时间间隔参数，通常是 `std::chrono` 命名空间中的某个时间类型（如 `std::chrono::seconds`、`std::chrono::milliseconds` 等）。
```cpp
int main() {
    std::this_thread::sleep_for(std::chrono::seconds(3));
}
```
在C++14中可以使用时间字面量，简化参数传递：
```cpp
	// 获取当前时间  
	std::time_t now = std::time(nullptr);
	std::cout << "当前系统时间是: " << now << endl;
	std::this_thread::sleep_for(3s);
	now = std::time(nullptr);
	std::cout << "当前系统时间是: " << now << endl;
```
执行结果：
```
当前系统时间是: 1719669301
当前系统时间是: 1719669304
```
**`yield`**： 提示调用线程放弃对 CPU 的使用权，以使其他线程可以运行。但是，这并不意味着调用线程会立即停止运行；它只是一个提示，操作系统可以选择忽略它。
```cpp
while (!isDone()){
    std::this_thread::yield();
}
```
线程需要等待某个操作完成，如果你直接用一个循环不断判断这个操作是否完成就会使得这个线程占满 CPU 时间，这会造成资源浪费。此时可以判断操作是否完成，如果还没完成就调用 yield 交出 CPU 时间片让其他线程执行，过一会儿再来判断是否完成，这样这个线程占用 CPU 时间会大大减少。
**`sleep_until`**：使当前线程阻塞，直到指定的时间点。与 `std::this_thread::sleep_for()` 类似，但它是基于绝对时间点的，而不是时间间隔。

## 线程资源所有权的转移
当通过线程对象的构造函数创建了一个关联了一个活跃线程的线程对象后，该对象就拥有了对该线程的独占所有权，同时线程对象`std::thread`是不可以复制的，每个线程对象对应一个唯一的线程(id不同)。
### 通过移动进行线程资源所有权转移
```
//std::thread转移所有权
//通过移动构造的方式
thread t1{ [] {std::cout << std::endl <<std::this_thread::get_id() << endl; } };
cout << "t1是否关联了活跃线程" << t1.joinable() << endl;
//将t1的线程资源所有权交给t2
thread t2{ move(t1) };
cout << "t1是否关联了活跃线程" << t1.joinable() << endl;
cout << "t2是否关联了活跃线程" << t2.joinable() << endl;
//t1.join();
t2.join();
```
执行结果如下：
```
t1是否关联了活跃线程1
t1是否关联了活跃线程0
t2是否关联了活跃线程1

21956
```
上面的代码通过`std::thread`的移动构造函数将`t1`的线程资源所有权转移给了`t2`，让持有线程资源的 `t2` 对象最后调用 `join()` 堵塞让其线程执行完毕。`t1` 与 `t2` 都能正常析构。
### 通过移动赋值操作进行线程资源所有权转移
```
//通过移动赋值的方式
thread t1;	//默认构造
cout << "t1是否关联了活跃对象" << t1.joinable() << endl;
thread t2{ [] {} };
t1 = move(t2);	//转移t2的线程资源给t1
cout << "t1是否关联了活跃对象" << t1.joinable() << endl;
cout << "t2是否关联了活跃对象" << t2.joinable() << endl;
t1.join();
//重新构造t2
t2 = thread{ [] {} };
cout << "t2是否关联了活跃对象" << t2.joinable() << endl;
t2.join();
```
执行结果如下：
```
t1是否关联了活跃对象0
t1是否关联了活跃对象1
t2是否关联了活跃对象0
t2是否关联了活跃对象1
```
上面代码中`t2 = thread{ [] {} };` `=`右边是临时对象，所以是右值，可以直接赋值无需`move()`
### 函数返回`std::thread`对象
```
//函数返回std::thread对象
std::thread f3()
{
	std::thread t{ [] {} };
	return t;
}

int main()
{
	//函数返回std::thread对象
	thread rt = f3();
	cout << "rt是否关联了活跃对象" << rt.joinable() << endl;
	rt.join();
	return 0;
}
```
上面代码中`f3()`创建了一个局部的`std::thread`对象，并将其返回，
在这里编译器选择了移动构造函数将`t`的线程资源所有权转移到返回用的临时`std::thread`对象，而此临时对象是右值，所以依然会调用移动构造函数将临时对象的线程资源所有权转移到`rt`，
然后`rt`正常调用`join()`。
上面代码的结果如下：
```
rt是否关联了活跃对象1
```
### 线程资源所有权在函数内部传递
```
//线程资源所有权在函数内部传递
void f4(std::thread t)
{
	std::cout << "子线程id" << std::this_thread::get_id() << std::endl;
	t.join();
}

int main()
{
	//线程资源所有权在函数内部传递
	thread t{ [] {} };
	//f4(t); //这是错误的
	f4(move(t));
	f4(thread{ [] {} });

	return 0;
}
```
首先`main()`函数中创建了一个线程对象`t`
利用`move()`将`t`转换为一个右值
调用`f4()`，通过移动构造将传入的右值初始化形参`t`，形参`t`获得线程资源所有权
函数中正常调用`join()`
`f4(thread{ [] {} });`中传入临时对象，临时对象作为右值会通过移动构造转移线程资源所有权

上面代码的运行结果如下：
```
子线程id24204
子线程id24204
```
## 共享数据的访问管理
当我们启用多个线程时，如果我们未对这些线程的执行次序进行限制，那么将由系统进行调度，那么这些线程的执行顺序将会是比较混乱的，如果这些线程的操作不涉及对共享数据的读写还好，但是如果涉及，那么就会出现`恶性条件竞争`。
```
std::vector<int>v;

void f() { v.emplace_back(1); }
void f2() { v.erase(v.begin()); }

int main() {
    std::thread t{ f };
    std::thread t2{ f2 };
    t.join();
    t2.join();
    std::cout << v.size() << '\n';
}
```
如上，两个线程共享一个`vector<int> v`，这时会出现很多问题，
比如 `f2` 先执行，此时 `vector` 还没有元素，导致抛出异常。又或者 `f` 执行了一半，调用了 `f2()`；当然了，也有可能先执行 f，然后执行 f2，最后打印了 0，程序老老实实执行完毕。
### 通过互斥量进行管理
互斥量(`mutex`)或者是互斥锁，通过设立一个标志位实现对于**临界资源**的互斥访问，而`mutex` 主要有两种状态，锁定(locked)与解锁(unlock)。
>当`mutex`处于`locked`状态时，一定有某个线程正持有这个锁，他正在访问**临界资源**
当`mutex`处于`unlock`状态时，则没有线程持有这个锁，没有线程正在访问**临界资源**

当没有使用互斥量时
```
//未加互斥量
void f5()
{
	std::cout << "此线程id:" << std::this_thread::get_id() << std::endl;
}

int main()
{
	//未加互斥量
	vector<thread> threads;
	//创建线程
	for (size_t i = 0; i < 10; i++)
	{
		threads.emplace_back(f5);
	}
	//启用线程
	for (auto& thread : threads)
	{
		thread.join();
	}
}

```
当你每次执行上面的代码时你会发现每次的结果都是不一样的(并不是指进程id)，顺序也毫无规律可言。
```
此线程id:此线程id:1248此线程id:1588
此线程id:20168此线程id:
此线程id:12864
此线程id:16476
此线程id:18936
此线程id:18628
此线程id:24488

22488
12736
```

```
此线程id:21336此线程id:此线程id:22952
此线程id:24340
此线程id:18276
此线程id:19612
此线程id:15792
此线程id:21684

16136
此线程id:17100
此线程id:11380
```
这里执行了两次。
当引入互斥量`mutex`后
```
//注意要引入必要的头文件
#include<mutex>
//使用互斥量
std::mutex m;
void f6()
{
	m.lock();
	std::cout << "此线程的id:" << std::this_thread::get_id() << std::endl;
	m.unlock();
}

int main()
{
	//使用互斥量
	vector<thread> threads;
	//创建线程
	for (size_t i = 0; i < 10; i++)
	{
		threads.emplace_back(f6);
	}
	//启用线程
	for (auto& thread : threads)
	{
		thread.join();
	}
}
```
这时会发现输出工整了很多：
```
此线程的id:24000
此线程的id:17652
此线程的id:22780
此线程的id:8224
此线程的id:21756
此线程的id:19600
此线程的id:24252
此线程的id:6984
此线程的id:23620
此线程的id:24404
```
这时当众多线程执行时都会先去获取锁`mutex`，但锁只有一个，所以只有一个线程会获得锁并继续执行后面的代码，而没有获得锁的线程会被阻塞直到获得锁，不会执行后面的代码
获得所得线程在执行`m.unlock()`后会释放锁，这时其他进程就可以获得锁了。
>  ！注意！
>  最后的`范围for循环`中`auto& thread : threads`要通过引用&的方式获取`threads`中的元素，
>  因为`std::thread`对象是不可复制的。
#### `std::lock_guard`
`std::lock_guard`是互斥量封装器，可以方便地进行互斥量的管理。
当你创建一个 `std::lock_guard` 对象时，它会尝试锁定指定的互斥量；当 `std::lock_guard` 对象离开其作用域（即被销毁）时，它会自动解锁互斥量。
这样就不用显式地调用`lock()`与`unlock()`，可以避免忘记手动`unlock()`而导致**死锁**
```
//std::lock_guard

std::mutex m;

//添加元素
void add_to_list(int n, std::list<int>& list)
{
	std::vector<int> numbers(n + 1);
	std::iota(numbers.begin(), numbers.end(), 0);
	int sum = std::accumulate(numbers.begin(), numbers.end(), 0);
	{
		std::lock_guard<std::mutex> lc{ m };
		list.push_back(sum);
	}
}
//输出元素
void print_list(const std::list<int>& list)
{
	std::lock_guard<std::mutex> lc{ m };
	for (const auto& i : list)
	{
		std::cout << i << ' ';
	}
	std::cout << std::endl;
}

int main()
{
	//std::lock_guard
	std::list<int> list;
	thread t1{ add_to_list,1,ref(list) };
	thread t2{ add_to_list,2,ref(list) };
	thread t3{ print_list,cref(list) };
	thread t4{ print_list,cref(list) };
	t1.join();
	t2.join();
	t3.join();
	t4.join();
}
```
输出结果如下：
```
3
3
```
当然结果不唯一，因为线程执行顺序不一致
`std::lock_guard`内部实现
```
//构造函数1
explicit lock_guard(_Mutex& _Mtx) : _MyMutex(_Mtx) { // construct and lock
        _MyMutex.lock();
    }
//构造函数2
lock_guard(_Mutex& _Mtx, adopt_lock_t) noexcept // strengthened
    : _MyMutex(_Mtx) {} // construct but don't lock
```
构造函数1接受一个互斥量，并用这个互斥量初始化其私有成员 `_MyMutex`，该成员用来引用互斥量
```
private:
    _Mutex& _MyMutex;
```

^74449c

可以看出，`std::lock_guard`在构造时就会去获取互斥量，进行`lock()`。
当然还有一个构造函数2，额外接受一个`std::adopt_lock_t`参数，此时构造函数不会上锁。
```
    ~lock_guard() noexcept {
        _MyMutex.unlock();
    }
```
其析构函数在被调用时，会释放互斥量，进行`unlock()`。
正式通过构造时上锁`lock()`，析构时解锁`unlock()`实现互斥量的管理。

同时可以通过`{ }`限制`std::lock_guard`的作用范围，这样进入作用域时自动上锁，离开作用域时析构销毁，自动解锁。如上面实例代码`add_to_list(int n, std::list<int>& list)`中的
```
	{
		std::lock_guard<std::mutex> lc{ m };
		list.push_back(sum);
	}
```

#### `try_lock()`
`try_lock()`是互斥量的一个成员函数，用这个函数获取锁时会对获取结果进行返回，如果获取失败并不会阻塞当前进程，而是会立即返回`false`，成功则返回`true`。
```
//try_lock
void thread_function(int id)
{
	//尝试获取锁
	if (m.try_lock())
	{
		std::cout << "线程" << std::this_thread::get_id() << "获得锁" << std::endl;
		//临界区代码
		std::this_thread::sleep_for(std::chrono::milliseconds(100));	//模拟临界区代码
		m.unlock();	//解锁
		std::cout << "线程" << std::this_thread::get_id() << "释放锁" << std::endl;
	}
	else
	{
		std::cout << "线程" << std::this_thread::get_id() << "获得锁失败" << std::endl;
	}

}

int main()
{
	//try_lock
	thread t1(thread_function, 1);
	thread t2(thread_function, 2);
	t1.join();
	t2.join();
}
```
执行结果如下：
```
线程5208获得锁线程19488获得锁失败

线程5208释放锁
```
当你既想多个线程互斥地访问**临界资源**，又不想进程因为没能获取锁而阻塞，可以使用`try_lock`
### 互斥量的保护失效的可能
使用互斥量可以实现对共享数据的保护，但是当你试图将保护的数据传递到互斥量保护范围之外时，互斥量的保护作用将失去作用。
```
//如果将受保护的数据的指针或引用传递到
//互斥量保护范围之外，互斥量的保护将无效
class Data
{
	int a{};
	std::string b{};
public:
	void updata_data()
	{
		//对数据成员做出修改
		std::cout << "Data对象的数据被修改了" << std::endl;
	}
};
class Data_wrapper
{
	Data data;
	std::mutex m;
public:
	template<class Func>
	//调用传递进来的函数
	void process_data(Func func)
	{
		//互斥量保护区域
		std::lock_guard<std::mutex> lc{ m };
		func(data);
	}
};
Data* p=nullptr;
//恶意函数，将受保护数据传出
void malicious_func(Data& protected_data)
{
	//将受保护数据传出
	p = &protected_data;
}
Data_wrapper d;
void foo()
{
	//传递恶意函数
	d.process_data(malicious_func);
	//非线程安全的修改数据
	p->updata_data();
}

int main()
{
	//如果将受保护的数据的指针或引用传递到
	//互斥量保护范围之外，互斥量的保护将无效
	foo();
}
```
`Data_wrapper`类有两个数据成员，分别是表示数据的`data`和互斥量`std::mutex m`，其成员函数`process_data()`使用互斥量保护对于传入的函数的调用。
但是当恶意地传入一个将受保护的数据传出的函数`malicious_func()`，这时就可以不受互斥量保护地修改应该受到保护的数据了。
结果如下：
```
Data对象的数据被修改了
```
当然上面的情况只是一种可能，**千万不要将受保护的数据的指针或引用传出互斥量的保护区域**。
### 死锁问题
当存在两个线程，每个线程都需要同时拥有资源A与资源B才能正常工作，而当一个线程只持有一个资源时，它会无限等待直到另一个资源被释放然后去获取那个资源。
这时就会容易出现死锁现象，如果线程1获取了资源A而线程2获取了资源B，这时两个线程都因为资源不全而无限等待另一个资源的释放，持有着资源却什么也不做，互相干瞪眼，而且这也会影响其他需要这些资源的线程。
这就是死锁。
比如：
```
std::mutex m1,m2;
std::size_t n{};

void f1(){
    std::lock_guard<std::mutex> lc1{ m1 };
    std::lock_guard<std::mutex> lc2{ m2 };
    ++n;
}
void f2() {
    std::lock_guard<std::mutex> lc1{ m2 };
    std::lock_guard<std::mutex> lc2{ m1 };
    ++n;
}
```
在上面的代码中有两个互斥量`m1 m2`,同时两个线程获取互斥量的顺序正好相反，这就容易出现`f1` 持有`m1`，`f2`持有`m2`，并且两个线程都在等待对方释放另一个资源的情形，也就是**死锁**。
那如果我统一互斥量获取的顺序呢？
```
struct X{
    X(const std::string& str) :object{ str } {}

    friend void swap(X& lhs, X& rhs);
private:
    std::string object;
    //互斥量
    std::mutex m;
};

void swap(X& lhs, X& rhs) {
    if (&lhs == &rhs) return;
    //先获取lhs.m，后获取rhs.m
    std::lock_guard<std::mutex> lock1{ lhs.m }; 
    std::lock_guard<std::mutex> lock2{ rhs.m }; 
    swap(lhs.object, rhs.object);
}

int main()
{
	X a{ "123" }, b{ "456" };
	std::thread t1{ [&] {swap(a,b); } };
	std::thread t2{ [&] {swap(b,a); } };
	t1.join();
	t2.join();
}
```
如上，即使互斥量的获取顺序统一了，但是在实际传参时参数顺序未必一致，这样就会可能出现
`t1`获取了`a`的互斥量而`t2`获取了`b`的互斥量而出现死锁。
> 但是我在`Visual Stidio`中运行好多次都正常运行了，可能VS进行了优化？ ^00001

这时可以通过`std::lock`解决，
`std::lock`在尝试获取多个互斥量时要么全部都获取到并继续执行代码，要么因为无法同时获取到全部互斥量会将所有以获取到的互斥量全部释放。
也就是说通过`std::lock`获取多个互斥量时，没有死锁风险。
新的代码如下：
```
//避免死锁可以用std::lock
struct X
{
	X(const std::string& str) :object{ str } {};
	friend void swap(X& lhs, X& rhs);
private:
	std::string object;
	std::mutex m;
};
void swap(X& lhs, X& rhs)
{
	//自引用检查
	if (&lhs == &rhs)
		return;
	//std::lock会同时获取多个互斥量
	//有一个无法获取，会将所有已获取的互斥量释放
	std::lock(lhs.m, rhs.m);
	std::lock_guard<std::mutex> l1{ lhs.m,std::adopt_lock };
	std::lock_guard<std::mutex> l2{ rhs.m,std::adopt_lock };
	std::swap(lhs.object, rhs.object);
	std::cout << "swap成功" << std::endl;
}

int main()
{
	//避免死锁可以用std::lock
	X a{ "123" }, b{ "456" };
	std::thread t1{ [&] {swap(a,b); } };
	std::thread t2{ [&] {swap(b,a); } };
	t1.join();
	t2.join();
	cout << "done" << endl;
}
```
在这里初始化`std::lock_guard`时使用的是不进行上锁的构造函数
```	
std::lock_guard<std::mutex> l1{ lhs.m,std::adopt_lock };
std::lock_guard<std::mutex> l2{ rhs.m,std::adopt_lock };
```
这是因为前面已经通过`std::lock(lhs.m, rhs.m)`获取过锁了，通过调用不上锁的构造函数就能正常调用析构释放锁了。
执行结果如下：
```
swap成功
swap成功
done
```

```
//C++17引用了std::scoped_lock提供了RAII包装
//不用自行解锁了
void swap(X& lhs, X& rhs)
{
	if (&lhs == &rhs)
		return;
	//std::lock会同时获取多个互斥量
	//有一个无法获取，会将所有已获取的互斥量释放
	std::scoped_lock guard(lhs.m, rhs.m);
	std::swap(lhs.object, rhs.object);
}
```
#### 关于避免死锁的建议
在多线程编程中，死锁问题经常是不可预见、难以稳定复现的，所以在编程时要多加注意。
1. 规定锁的获取顺序，防止无限等待锁的释放；
2. 避免嵌套锁，尽量在持有锁时不要再去获取锁；
3. 设置等待超时，当在等待锁时设置超时，不要无限等待；
4. 限制锁的保护范围，只保护需要保护的数据，比如通过`{}`进行范围限定；
5. 避免在持有锁时调用外部代码，因为外部代码可能或作出线程危险的操作。
### `std::unique_lock`
类 `unique_lock` 是一种通用互斥包装器，允许延迟锁定、有时限的锁定尝试、递归锁定、所有权转移和与条件变量一同使用。
`std::unique_lock`是一种灵活的锁，首先看一下内部结构：
```
private:
    _Mutex* _Pmtx = nullptr;
    bool _Owns    = false;
```
首先是内部数据成员，一个互斥量指针`_Pmtx`和一个表示是否拥有对于互斥量所有权的`bool`型变量`_Owns`。
构造函数
```
unique_lock(_Mutex& _Mtx, defer_lock_t) noexcept
    : _Pmtx(_STD addressof(_Mtx)), _Owns(false) {} // construct but don't lock
```
可以看到，构造函数仅仅是对两个数据成员进行了初始化，并且`_Owns`设置为了`false`，即**初始是没有互斥量的所有权的**。
成员函数`lock()`
```
void lock() { // lock the mutex
    _Validate();
    _Pmtx->lock();
    _Owns = true;
}
```
可以看到，在`lock()`中会对互斥量上锁，并将所有权`_Owns`设置为`true`，即获得了所有权。
析构函数
```
~unique_lock() noexcept {
    if (_Owns) {
        _Pmtx->unlock();
    }
}
```
**在析构中，会进行所有权的检查，如果拥有所有权即在此之前`lock()`过，则会解锁`unlock()`互斥量。**
当向构造函数中传入参数`std::adopt_lock`，会发生什么？
```
std::mutex m;

int main() {
    std::unique_lock<std::mutex> lock{ m,std::adopt_lock };
    lock.lock();
}
```
这样会报错的，因为`std::adopt_lock`会获得所有权但不上锁，所以调用`lock()`时，`_Owns`已经是`true`了，所以在`lock()`中的检测步骤`_Validate()`中会出错。
```
void _Validate() const { // check if the mutex can be locked
    if (!_Pmtx) {
        _Throw_system_error(errc::operation_not_permitted);
    }

    if (_Owns) {
        _Throw_system_error(errc::resource_deadlock_would_occur);
    }
}
```
所以在调用`std::unique_lock`的`lock()`时要保证没有互斥量的所有权。
可以这样写：
```
int main()
{
	std::mutex m;
	m.lock();
	std::unique_lock<std::mutex> lock{ m,std::adopt_lock };
	std::cout << "成功上锁" << std::endl;
	return 0;
}
```
那这个**灵活的锁**到底灵活在哪？
1.延迟锁定和解锁
因为`std::unique_lock`在构造时并不会立刻获得锁的所有权，所以你可以在你需要的地方进行锁定和解锁
```
std::mutex mtx; 
std::unique_lock<std::mutex> lck(mtx, std::defer_lock); // 延迟锁定 
// ... 
lck.lock(); // 在需要时锁定 
// ... 
lck.unlock(); // 手动解锁
```
2.所有权转移
这里同上面的线程资源所有权转移，即通过移动转移所有权。
```
std::mutex mtx;
std::unique_lock<std::mutex> lck1(mtx);
std::unique_lock<std::mutex> lck2 = std::move(lck1); // 转移所有权和锁定状态
```
同样可以在不同作用域中进行所有权转移
```
std::unique_lock<std::mutex>get_lock(){
    extern std::mutex some_mutex;
    std::unique_lock<std::mutex> lk{ some_mutex };
    return lk;

}
void process_data(){
    std::unique_lock<std::mutex> lk{ get_lock() };
    // 执行一些任务...
}
```
`get_lock()`会创建一个`std::unique_lock`并将其返回，因为锁也是不可复制的，所以是通过移动构造实现的所有权转移。
> `extern`使`some_mutex`具有静态或线程存储期，确保`process_data()`中的`lk`不会悬垂。

3.条件变量
  当与`std::condition_variable`一起使用时，`std::unique_lock`特别有用。你可以使用`std::unique_lock`来锁定互斥量，并在等待条件变量时自动释放锁，然后在条件变量被唤醒时重新获取锁。 ^00002
```
 std::mutex mtx; 
 std::condition_variable cv; 
 std::unique_lock<std::mutex> lck(mtx); 
 cv.wait(lck, []{ /* 条件函数 */ }); // 等待时释放锁，唤醒时重新获取锁 
```
4.可尝试锁定
`std::unique_lock`还提供了`try_lock()`方法，该方法尝试锁定互斥量，如果互斥量已经被另一个线程锁定，则立即返回而不会阻塞。也就是有时限的锁定尝试。
```
std::mutex mtx; 
std::unique_lock<std::mutex> lck(mtx, std::try_to_lock); 
// 尝试锁定 
if (lck.owns_lock()) 
{ 
	// 成功锁定 
} 
else 
{ 
	// 互斥量已经被另一个线程锁定 
}
```
### 其他保护共享数据的方法
#### 双检锁
```
void f(){
    if(!ptr){      // 1
        std::lock_guard<std::mutex> lk{ m };
        if(!ptr){  // 2
            ptr.reset(new some);  // 3
        }
    }
    ptr->do_something();  // 4
}
```
第一次检查，检查共享资源是否被初始化，如果没有则获取锁，准备进行初始化。
第二次检查，再次检查共享资源是否被初始化，避免其他线程在第一次检查后进行了初始化，否则后面对资源进行初始化时可能会重复初始化。
但其实这里还存在着**潜在的条件竞争**，因为对共享资源的初始化操作并非不可中断的操作，如果另一个相同的线程在这个线程只是分配内存对指针写入时执行，则不会进入第一个if，会直接执行`ptr->do_something()`，但此时对象还未构造，会出现错误。
所以就有了下面的`std::call_once`
#### **`std::call_once`**
比起锁住互斥量并显式检查指针，每个线程只需要使用 `std::call_once` 就可以。
 要注意`std::call_once`要与`std::once_flag`配合使用，同时通过`std::call_once`初始化只会调用一次，除非抛出了异常。
```
//std::call_once与std::once_flag
std::once_flag flag1, flag2;
//正常情况下call_once只会调用一次
void do_once()
{
	std::call_once( flag1, [] {std::cout << "do_once只会调用一次" << std::endl; } );
};
void may_throw(bool flag)
{
	if (flag)
	{
		std::cout << "may_do_times会再次调用" << std::endl;
		throw std::exception();
	}
	std::cout << "may_do_times不会再调用" << std::endl;
}
//除非抛出异常
void may_do_times(bool flag)
{
	try
	{
		std::call_once(flag2, may_throw, flag);
	}
	catch (const std::exception&)
	{}
}

int main()
{
	//正常情况下call_once只会调用一次
	std::thread t1{ do_once };
	std::thread t2{ do_once };
	t1.join();
	t2.join();
	//除非抛出异常
	std::thread t3{ may_do_times,true };
	std::thread t4{ may_do_times,true };
	std::thread t5{ may_do_times,false };
	t3.join();
	t4.join();
	t5.join();

	return 0;
}
```
一种结果如下：
```
do_once只会调用一次
may_do_times会再次调用
may_do_times不会再调用
```
虽然有两个线程调用`do_once`但只执行了一次，
但如果抛出了异常，则可以重复执行。
#### 静态局部变量初始化是线程安全的
```
class my_class;
inline my_class& get_my_class_instance(){
    static my_class instance;  // 线程安全的初始化过程 初始化严格发生一次
    return instance;
}
```
即使多个线程调用`get_my_class_instance()`,也只有一个线程进行了`instance`的初始化，其他线程会等待初始化完成。
## 参考资料
https://github.com/Mq-b/ModernCpp-ConcurrentProgramming-Tutorial