#include "pplx_task.hpp"
#include <unistd.h> 

int main()
{
    // 问题: Finally value 会先执行 
    //auto str = write_to_string();
    //std::cout << "Finally value" << std::endl;

    //test1
    /*
    auto t1 = create_task_test(); 
    auto result = t1.get();
    std::cout << result << std::endl;
    auto t2 = pplx::create_task([]{
        std::string str("wohahaha");
        return str;
    });
    std::cout << t2.get() << std::endl;
    */

    
    //test2: test chain
    /*
    task_chains();
    sleep(1);
    std::cout << "exec end" << std::endl;
    */

    // test3: test group 
    task_group([]() {
        std::cout << "hello world" << std::endl;
        sleep(2);
    },3);



    return 0;
}
