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
    /*
    task_group([]() {
        std::cout << "hello world" << std::endl;
        sleep(2);
    },3);
    */


    //test4: test task group for reading file 
    FILE *fd = fopen("/Users/pengbin/temp/douban.txt","rb");
    fseek(fd, 0, SEEK_SET);
    struct BlockInfo {
        int block_index;
        std::vector<char> block_data;
    };
    auto generate_block_info = [=, file_mutex = std::make_shared<std::mutex>(), index = std::make_shared<int>(0),
                                block_size = 16]() ->  std::shared_ptr<BlockInfo> {
        std::unique_lock<std::mutex> lock(*file_mutex);

        if(::feof(fd))
        {
            std::cout << "reach end of the file" << std::endl;
            return nullptr;
        }
        auto result = std::make_shared<BlockInfo>();
        result->block_index = (*index);
        result->block_data.resize(block_size);

        size_t ret;
        ret = ::fread(result->block_data.data(), 1,block_size, fd);
        std::cout << "read size is: " << ret << std::endl;

        for(auto& it : result->block_data)
        {
            std::cout << it;
        }

        ++(*index);
        lock.unlock();

        return result;
    };

    task_group_read_file([=]() {
        auto block_info = generate_block_info();
        return block_info ? true : false;
    },
    3);



    return 0;
}
