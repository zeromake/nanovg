#include <rxcpp/rx.hpp>
#include <thread>
#include <experimental/coroutine>

int main(int argc, char*argv[]) {
    rxcpp::schedulers::run_loop runloop;
    printf("main: %d\n", std::this_thread::get_id());
    auto observable = rxcpp::observable<void, void>::create<int>([](rxcpp::subscriber<int> subscriber) {
        printf("subscriber: %d\n", std::this_thread::get_id());
        for (int i = 0; i < 5; i++) {
            subscriber.on_next(i);
        }
    });
    auto scheduler = rxcpp::identity_current_thread();
    auto scheduler2 = rxcpp::observe_on_run_loop(runloop);
    observable
        .subscribe_on(scheduler2)
        .observe_on(scheduler2)
        .subscribe([](int i) {
            printf("observe: %d\n", std::this_thread::get_id());
            printf("out: %d\n", i);
        });
    // _sleep(1000);
    bool runlooping = true;
    std::thread runloopThread([&runloop, &runlooping]() {
        printf("runloopThread: %d\n", std::this_thread::get_id());
        while (runlooping) {
            if (!runloop.empty())
                runloop.dispatch();
        }
    });
    while (!runloop.empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }
    printf("exit\n");
    runlooping = false;
    runloopThread.join();
    return 0;
}
