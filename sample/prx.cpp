
#include "liborbisutil.h"

#include <assert.h>
#include <csetjmp>
#include <iostream>

#include "minhook.h"

// We need to provide an export to force the expected stub library to be generated
__declspec (dllexport) void dummy()
{

}

// Test Case for liborbisutil::string
class TestCase {
public:
    TestCase(const std::string& name) : test_name(name) {}

    virtual void run() = 0;

    std::string get_name() const {
        return test_name;
    }

protected:
    std::string test_name;
};

// Test Case for liborbisutil::string
class LibOrbisUtilStringTest : public TestCase {
public:
    LibOrbisUtilStringTest() : TestCase("liborbisutil_string_test") {}

    void run() override {
        test_to_lower();
        test_to_upper();
        test_to_wide();
        test_to_narrow();
        test_begins_with();
        test_ends_with();
        test_contains();
        test_contains_case_insensitive();
        test_split();
        test_join();
        test_reverse();
        test_replace();
        test_erase();
        test_vector_to_pointer_array();
        test_pointer_array_to_vector();
        test_is_numeric();
        test_is_integer();
        test_is_alphabetic();
    }

private:
    void print_result(const std::string& test_name, bool passed) {
        std::cout << test_name << ": " << (passed ? "PASSED" : "FAILED") << "\n";
    }

    void test_to_lower() {
        auto str = liborbisutil::string::to_lower("Hello, World!");
        print_result("test_to_lower", str == "hello, world!");
    }

    void test_to_upper() {
        auto str = liborbisutil::string::to_upper("Hello, World!");
        print_result("test_to_upper", str == "HELLO, WORLD!");
    }

    void test_to_wide() {
        auto wide = liborbisutil::string::to_wide("Hello, World!");
        print_result("test_to_wide", wide == L"Hello, World!");
    }

    void test_to_narrow() {
        auto wide = liborbisutil::string::to_wide("Hello, World!");
        auto narrow = liborbisutil::string::to_narrow(wide);
        print_result("test_to_narrow", narrow == "Hello, World!");
    }

    void test_begins_with() {
        auto result = liborbisutil::string::begins_with("Hello, World!", "Hello");
        print_result("test_begins_with", result == true);
    }

    void test_ends_with() {
        auto result = liborbisutil::string::ends_with("Hello, World!", "World!");
        print_result("test_ends_with", result == true);
    }

    void test_contains() {
        auto result = liborbisutil::string::contains("Hello, World!", "World");
        print_result("test_contains", result == true);
    }

    void test_contains_case_insensitive() {
        auto result = liborbisutil::string::contains("Hello, World!", "world", false);
        print_result("test_contains_case_insensitive", result == true);
    }

    void test_split() {
        auto strings = liborbisutil::string::split("Hello, World!", ",");
        bool passed = strings.size() == 2 && strings[0] == "Hello" && strings[1] == " World!";
        print_result("test_split", passed);
    }

    void test_join() {
        std::vector<std::string> strings = { "Hello", "World!" };
        auto str = liborbisutil::string::join(strings, " ");
        print_result("test_join", str == "Hello World!");
    }

    void test_reverse() {
        auto str = liborbisutil::string::reverse("Hello, World!");
        print_result("test_reverse", str == "!dlroW ,olleH");
    }

    void test_replace() {
        auto str = liborbisutil::string::replace("Hello, World!", "World", "Orbis");
        print_result("test_replace", str == "Hello, Orbis!");
    }

    void test_erase() {
        auto str = liborbisutil::string::erase("Hello, World!", "World");
        print_result("test_erase", str == "Hello, !");
    }

    void test_vector_to_pointer_array() {
        std::vector<std::string> strings = { "Hello", "World!" };
        auto pointer_array = liborbisutil::string::vector_to_pointer_array(strings);
        bool passed = std::string(pointer_array[0]) == "Hello" &&
            std::string(pointer_array[1]) == "World!" &&
            pointer_array[2] == nullptr;
        print_result("test_vector_to_pointer_array", passed);
    }

    void test_pointer_array_to_vector() {
        const char* pointer_array[] = { "Hello", "World!", nullptr };
        auto vector = liborbisutil::string::pointer_array_to_vector(const_cast<char**>(pointer_array), 2);
        bool passed = vector.size() == 2 && vector[0] == "Hello" && vector[1] == "World!";
        print_result("test_pointer_array_to_vector", passed);
    }

    void test_is_numeric() {
        auto result = liborbisutil::string::is_numeric("123");
        print_result("test_is_numeric", result == true);
    }

    void test_is_integer() {
        auto result = liborbisutil::string::is_integer("123");
        print_result("test_is_integer", result == true);
    }

    void test_is_alphabetic() {
        auto result = liborbisutil::string::is_alphabetic("Hello");
        print_result("test_is_alphabetic", result == true);
    }
};

void run_log_tests()
{
    LOG_INFO("This is an info message\n");
    LOG_ERROR("This is an error message\n");
    LOG_WARNING("This is a warning message\n");
    LOG_SUCCESS("This is a success message\n");
    LOG_DEBUG("This is a debug message\n");
}

void run_thread_tests()
{
    liborbisutil::thread_pool::initialize();

    auto work = liborbisutil::thread_pool::push([]() {
        LOG_INFO("Worker thread is running\n");
        });

    work.get();

    liborbisutil::thread_pool::finalize();

    liborbisutil::threading::create("test_thread", [](void* arg) -> void* {
        LOG_INFO("Thread is running\n");

        return nullptr;
        }, true);

    liborbisutil::thread thr([]() {

        LOG_INFO("new thread is running\n");
        }, "test");

    thr.join();

    auto all_threads = liborbisutil::threading::get_threads();

    LOG_INFO("All threads:\n");
    for (auto thr : all_threads)
    {
        LOG_INFO("\t[%i]%s\n", thr->tid, liborbisutil::threading::get_name(thr).c_str());
    }

    auto current_thread = liborbisutil::threading::get_current();
    LOG_INFO("Current thread: %s\n", liborbisutil::threading::get_name(current_thread).c_str());

    auto thread_by_id = liborbisutil::threading::get_by_id(all_threads[0]->tid);
    LOG_INFO("Thread by ID: %s\n", liborbisutil::threading::get_name(thread_by_id).c_str());

    auto thread_by_name = liborbisutil::threading::get_by_name(liborbisutil::threading::get_name(all_threads[0]));
    LOG_INFO("Thread by name: %s\n", liborbisutil::threading::get_name(thread_by_name).c_str());
}

void run_http_tests()
{
    liborbisutil::http::initialize();

	auto response = liborbisutil::http::get("https://httpbin.org/get");
	LOG_INFO("GET Response: %s\n", response.data.c_str());
	response = liborbisutil::http::post("https://httpbin.org/post", "Hello, World!");
	LOG_INFO("POST Response: %s\n", response.data.c_str());
	response = liborbisutil::http::download("https://httpbin.org/image/png", "/data/test.png");
    if(response.success)
	{
		LOG_INFO("Downloaded image to /data/test.png\n");
	}
	else
	{
		LOG_ERROR("Failed to download image\n");
	}

    // put
	response = liborbisutil::http::put("https://httpbin.org/put", "Hello, World!");
	LOG_INFO("PUT Response: %s\n", response.data.c_str());

	// delete
	response = liborbisutil::http::del("https://httpbin.org/delete");
	LOG_INFO("DELETE Response: %s\n", response.data.c_str());

    liborbisutil::http::finalize();
}

void run_sockets_test()
{
	liborbisutil::socket sock("httpbin.org", 80);

    if (sock.connect())
    {
		LOG_INFO("Connected to server\n");

		// create post request
		std::string request = "GET /get HTTP/1.1\r\nHost: httpbin.org\r\nConnection: close\r\n\r\n";

		// send request
		sock.send(request.data(), request.size());

		// receive response
		char buffer[2048];
		sock.receive(buffer, sizeof(buffer));

		LOG_INFO("Response:\n%s\n", buffer);

		sock.close();
    }
    else
    {
        LOG_ERROR("Failed to connect to server\n");
    }
}

liborbisutil::hook::manager hook_manager;

void function_original()
{
    LOG_INFO("Function is original\n");
}

void function_hook()
{
	LOG_INFO("Function is hooked\n");

    hook_manager.invoke("function_original");
}

extern "C" 
{
	int module_start(size_t argc, const void* args)
	{
        LibOrbisUtilStringTest test;
        std::cout << "Running " << test.get_name() << "...\n";
        test.run();
        std::cout << "All tests passed!\n";

		liborbisutil::thread thr([]() {

            //run_log_tests();
            //run_thread_tests();

            //run_http_tests();
            //run_sockets_test();

            MH_Initialize();

			hook_manager.create("function_original", reinterpret_cast<void*>(function_original), function_hook);

            function_original();
            hook_manager.clear();

			MH_Uninitialize();

			}, "tests");

        thr.join();

		return 0;
	}

	int module_stop(size_t argc, const void* args)
    {
		return 0;
	}
}