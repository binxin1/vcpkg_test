#include <stdio.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <vector> 
#include <string>
#include <utility>
#include <iostream>

#include <sys/stat.h> 

#include <shared_mutex> 
#include <unistd.h>
#include <thread>


/*
    1> 定时器执行任务 & 定时获取当前文件的上传进度
    2> 定时器单独来进行查询
*/

/*
    1> 对IO读写的封装; 
        static MimeData::readcallback
        static MimeData::seekcallback
    2> progressGuard 
        progressGuard::_current 
        progressGuard::update 


    3> progressManager
        progressManager::sub : 包含了 progressGuard 的集合 
        progressManager::create_guard : 
        progressGuard::Dump_current : 定时任务，定时对 sub 来进行遍历，每次遍历时，获取 progressGuard::_current 的值; 


    如果添加进来，则需要单独启动一个线程来用于定时器; 
    而定时器每次需要执行的函数就是  progressGuard::Dump_current 


    thread1 : 文件读取时，需要更新 update 
    thread2 : 定时查询 update 的值
    锁的来源 
*/


using callback_func = std::function<void (int delta)>;


class ProgressManager;
class ProgressGuard {
public:
    ProgressGuard(std::shared_ptr<std::shared_mutex> mutex)
        :mutex_(mutex), current_(0){}

    void update(int delta)
    {
        std::unique_lock<std::shared_mutex> lock(*mutex_);
        //std::cout << "current= " << current_ << "delta= " << delta << std::endl;
        current_ += delta;
    }

    int get_current() const
    {
        return current_;
    }
private: 
    std::shared_ptr<std::shared_mutex> mutex_;
    int current_;
};



class ProgressManager {
public: 
    ProgressManager() 
        : mutex_(std::make_shared<std::shared_mutex>())
    {}

    void timer_func()
    {
        int last = 0; 
        while(true)
        {
            std::cout << "hello world size = " << subs_.size() << std::endl;
            sleep(1);
            {
                std::unique_lock<std::shared_mutex> info_lock(*mutex_);
                for(auto ptr_it = subs_.begin(); ptr_it != subs_.end(); ++ptr_it)
                {
                    int current = (*ptr_it)->get_current();
                    int diff = current - last;
                    std::cout << "current= " << current << "speed= " << diff << std::endl;
                }
            }
        }
    }

    std::shared_ptr<ProgressGuard> create_guard()
    {
        std::unique_lock<std::shared_mutex> lock(*mutex_);
        auto progress_guard = std::make_shared<ProgressGuard>(mutex_);
        subs_.push_back(progress_guard);

        std::cout << "create_guard: add new one size: "<< subs_.size() << std::endl;

        return progress_guard;
    }
    
//private: 
public:
    std::vector<std::shared_ptr<ProgressGuard>> subs_; 
    mutable std::shared_ptr<std::shared_mutex> mutex_; 
};



int main()
{    
    CURL *easy_handle;
    CURLcode res;

    struct stat file_info;
    curl_off_t speed_upload, total_time;
    FILE *fd = NULL;
    int file_size; 
    struct stat st;
    stat("/Users/pengbin/temp/douban.txt",&st);
    file_size = st.st_size;
    fd = fopen("/Users/pengbin/temp/douban.txt", "rb");
    
    if(!fd)
    {
        printf("Open file error");
        return 1;
    }
    if(fstat(fileno(fd), &file_info) != 0)
    {
        printf("fstat error\n");
    }

    std::string url("http://127.0.0.1:8000"); 
    CURLU *url_handle = curl_url();
    curl_url_set(url_handle, CURLUPART_URL, url.c_str(), 0);    
    char *real_url = nullptr;
    curl_url_get(url_handle, CURLUPART_URL, &real_url, 0);

    easy_handle = curl_easy_init();


    ProgressManager progress_manager_;
    std::thread timer_thread(&ProgressManager::timer_func, std::ref(progress_manager_));
    std::shared_ptr<ProgressGuard> progress_guard = progress_manager_.create_guard(); 

    std::cout << progress_manager_.subs_.size() << std::endl;

    if(easy_handle)
    {
        curl_easy_setopt(easy_handle, CURLOPT_URL, real_url);
        curl_easy_setopt(easy_handle, CURLOPT_READDATA, fd);
        curl_easy_setopt(easy_handle, CURLOPT_INFILESIZE_LARGE,
        (curl_off_t)file_info.st_size);

        curl_easy_setopt(easy_handle, CURLOPT_VERBOSE, 1L);

        class MimeData {
        private: 
            FILE* fd_stream; 
            callback_func callback_;

        public: 
            //此处的callback_func 不能是引用
            //实际项目是传递给它是已经是第二级了，第一级是 direct_upload_file，就不是引用
            MimeData(FILE* file_stream, callback_func callback)
              : fd_stream(file_stream), callback_(callback) {}


            static size_t read_callback(char* buffer, size_t size, size_t nitems, void *arg)
            {
                
                auto thiz = reinterpret_cast<MimeData*>(arg);
                auto delat = fread(buffer, size, nitems, thiz->fd_stream); 

                //std::cout << "read_callback delat= " << delat << std::endl; 

                thiz->callback_(delat);
                return delat;
            }

            static int seek_callback(void* arg, curl_off_t offset, int origin)
            {
                bool result = false;
                auto thiz = reinterpret_cast<MimeData*>(arg);
                result = fseek(thiz->fd_stream, offset, origin);

                return result ? CURL_SEEKFUNC_OK : CURL_SEEKFUNC_FAIL; 
            }
        };
        MimeData mime_data(fd, [progress_guard](int delta){ progress_guard->update(delta); });
        
        //set MIME
        auto mime_handle = curl_mime_init(easy_handle);
        curl_mimepart* part = curl_mime_addpart(mime_handle);
        curl_mime_name(part, "file");
        curl_mime_filename(part, "douban.txt");
        curl_mime_type(part, "application/octet-stream");
        // 注意此处参数的设置: fd
        // 此处的size 需要设置文件的大小 
        curl_mime_data_cb(part, file_size, MimeData::read_callback, MimeData::seek_callback, NULL, &mime_data);
        curl_easy_setopt(easy_handle, CURLOPT_MIMEPOST, mime_handle);
        res = curl_easy_perform(easy_handle);


        if(res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
            printf("error");
        }
        else
        {
            /* now extract transfer info*/
            curl_easy_getinfo(easy_handle, CURLINFO_SPEED_UPLOAD_T, &speed_upload);
            curl_easy_getinfo(easy_handle, CURLINFO_TOTAL_TIME_T, &total_time);
            fprintf(stderr, "Speed: %" CURL_FORMAT_CURL_OFF_T "bytes/sec during %"
            CURL_FORMAT_CURL_OFF_T " .%06ld seconds\n",
                speed_upload,
                (total_time / 1000000), (long)(total_time % 100000));
                }
            curl_easy_cleanup(easy_handle);
        }
    fclose(fd);

    timer_thread.join();
    
    return 0;
}