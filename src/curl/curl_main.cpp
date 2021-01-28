#include <stdio.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <vector> 
#include <string>
#include <utility>
#include <iostream>


char hugedata[512000];
struct ctl {
    char* buffer; 
    curl_off_t size;
    curl_off_t position;
};




size_t read_callback(char* buffer, size_t size, size_t nitems, void *arg)
{

    std::cout << "read_callback " << std::endl;

    FILE *p = (FILE *)arg;
    size_t read_size = 0;
    read_size = fread(buffer,size, nitems, p);
    return read_size;
}

int seek_callback(void* arg, curl_off_t offset, int origin)
{
    bool result = false;
    FILE *p = (FILE *)arg;
    result = fseek(p, offset, origin);

    return result ? CURL_SEEKFUNC_OK : CURL_SEEKFUNC_FAIL; 
}



int main()
{    
    CURL *easy_handle;
    CURLcode res;

    struct stat file_info;
    curl_off_t speed_upload, total_time;
    FILE *fd = NULL;
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

    if(easy_handle)
    {
        curl_easy_setopt(easy_handle, CURLOPT_URL, real_url);

        //curl_easy_setopt(easy_handle, CURLOPT_UPLOAD, 1L);
        //curl_easy_setopt(easy_handle, CURLOPT_POST, 1L);
        curl_easy_setopt(easy_handle, CURLOPT_READDATA, fd);
        curl_easy_setopt(easy_handle, CURLOPT_INFILESIZE_LARGE,
        (curl_off_t)file_info.st_size);

        curl_easy_setopt(easy_handle, CURLOPT_VERBOSE, 1L);
        
        


        //set MIME
        auto mime_handle = curl_mime_init(easy_handle);
        curl_mimepart* part = curl_mime_addpart(mime_handle);
        curl_mime_name(part, "file");
        curl_mime_filename(part, "douban.txt");
        curl_mime_type(part, "application/octet-stream");
        struct ctl hugectl; 
        
    
        curl_mime_data_cb(part, 6114, read_callback, seek_callback, NULL, fd);

        
        //curl_slist *curl_headers = NULL;
        //curl_headers = curl_slist_append(curl_headers, "Content-Type: multipart/form-data");
        //curl_easy_setopt(easy_handle, CURLOPT_HTTPHEADER, curl_headers);

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
    return 0;
}