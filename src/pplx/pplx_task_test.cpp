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



    /*
    test
        这里sleep 的原因是 "等待"
    */
    task_chains();
    sleep(1);
    std::cout << "exec end" << std::endl;
    return 0;
}
