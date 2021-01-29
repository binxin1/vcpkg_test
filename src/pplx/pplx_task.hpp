#include <pplx/pplxtasks.h>
#include <iostream>
#include <string>
#include <fstream> 
#include <vector>


pplx::task<std::string> write_to_string()
{
    //std::string s = std::make_shared<std::string>("Value 1");
    std::string s = "Value 1";

    return pplx::create_task([s] 
    {
        // Print the current value.
        std::cout << "[make1]Current value: " << s << std::endl;
        
    }).then([s] 
    {
        // Print the current value.
        std::cout << "[make2]Current value: " << s << std::endl;
        return s;
    });
}


pplx::task<std::string> write_to_string_task_chain()
{
    std::string s = "Value 1";
    
    return pplx::create_task([s]() {
        std::cout <<"[mark1]Current value:" << s << std::endl;
        return std::string("Value 22222");
    })

    .then([](std::string value) {
        std::cout << "[mark2]Current value:" << value << std::endl;
        return std::string("Value 3");
    })

    .then([](std::string value) {
        std::cout << "[mark3]Current value:" << value << std::endl;
        return std::string("Value 4");
    });
}



pplx::task<std::string> create_task_test()
{
    std::string str("hello world");
    return pplx::create_task([str]{
            std::cout << str << std::endl;
            return str;
    });
}
pplx::task<void> call_create_task_test()
{
    std::cout << "call_create_task_test " << std::endl;
    bool flag = true;
    if(flag)
    {
        //return create_task_test();
    }
}




/*
第一个create_task 
    1> 内部包含一个 create_task;
    2> 包含两个 return ,第一个return 返回的是task, 第二个 return 返回的是 string 
    3> 从测试的结果看，第一个的return用于第一个 then 的输入，第二个return 用于第二个then 的输入。



*/
void task_chains()
{
    std::string str("hello world");
    pplx::create_task([str]() {
        // level2 
        return pplx::create_task([str]() {
            // level1 
            return pplx::create_task([str]() {
                std::cout << "[first]str: "<< str << std::endl;
                // 此 then 作为下面的这个 then 的输入
                return str;
            })
            .then([=](std::string str) {
                std::cout << "hello world: "<< str << std::endl;
                str = str + "first then";
                // 注意: 此处需要返回一个 str ，用于下一个 then 的输入。 
                return str;
            });
        });
    })
    .then([=](std::string str) {
        std::cout << "[out]str: "  << str <<std::endl; 
    })
    // 如果将 void 修改为 std::string，那么使用一个 then 就可以处理。 
    // 并且可以使用  cout << pplx_task.get() << endl
    .then([=](pplx::task<void> pplx_task){
        std::cout << "second then" << std::endl;
        pplx_task.get();
    });
}



/*
    test group 
*/
template <typename F>
void task_group(F func, int count)
{
    std::vector<pplx::task<void>> tasks;
    for(int i = 0; i < count; ++i)
    {
        tasks.emplace_back(pplx::create_task([=]() {
            func();
        }));
    }

    for(auto task :tasks)
    {
        task.get();
        std::cout << "exec end" << std::endl;
    }
}


