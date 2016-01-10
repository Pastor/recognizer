#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

#include <allheaders.h>
#include <capi.h>

static std::mutex _p_queue_mutex;
static std::mutex _p_worker_mutex;
static std::queue<std::string> _p_queue;
static volatile bool _p_runable = true;
static std::thread _p_worker;

void save_content(const char * const hash, const std::string &content);

static bool 
has_and_get(std::string &hash)
{
    std::lock_guard<std::mutex> locker(_p_queue_mutex);
    if (_p_queue.empty())
        return false;
    hash = _p_queue.front();
    _p_queue.pop();
    return true;
}

static void
recognizer_thread()
{
    TessBaseAPI *handle = nullptr;
    int          ret;

    handle = TessBaseAPICreate();
    ret = TessBaseAPIInit3(handle, "", "rus");
    if (ret != 0) {
        return;
    }
    std::chrono::milliseconds timeout(500);
    while (_p_runable) {
        std::string hash;

        if (has_and_get(hash)) {
            auto start = std::chrono::high_resolution_clock::now();
            {
                std::lock_guard<std::mutex> locker(_p_worker_mutex);
                PIX         *pix = nullptr;

                if ((pix = pixRead(hash.c_str())) != nullptr) {
                    TessBaseAPISetPageSegMode(handle, PSM_AUTO_ONLY);
                    TessBaseAPISetImage2(handle, pix);
                    TessBaseAPISetInputName(handle, hash.c_str());
                    ret = TessBaseAPIRecognize(handle, nullptr);
                    if (ret == 0) {
                        auto text = TessBaseAPIGetUTF8Text(handle);
                        save_content(hash.c_str(), text);
                        TessDeleteText(text);
                    }
                    pixDestroy(&pix);
                }
            }
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elapsed = end - start;
            fprintf(stdout, "%s: %f ms\n", hash.c_str(), elapsed.count());
        } else {
            std::this_thread::sleep_for(timeout);
        }
    }
    TessBaseAPIEnd(handle);
    TessBaseAPIDelete(handle);
}

void
recognizer_push(const char * const hash)
{
    std::lock_guard<std::mutex> locker(_p_queue_mutex);
    _p_queue.push(hash);
    fprintf(stdout, "Pushed %s\n", hash);
    fprintf(stdout, "Size   %d\n", _p_queue.size());
}

void
recognizer_start()
{
    if (_p_worker.get_id() == std::thread::id()) {
        _p_worker = std::thread(recognizer_thread);
        //_p_worker.detach();
    }
}



