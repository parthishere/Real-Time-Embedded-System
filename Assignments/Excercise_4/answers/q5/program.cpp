#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <string>
#include <map>
#include <cstdlib> // For std::exit and EXIT_FAILURE
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <fstream>

#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <aio.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sched.h>
#include <stddef.h>
#include <sys/sysinfo.h>
#include <semaphore.h>

#define MAX_FRAME 10
#define NUM_OF_THREAD 2
#define ESCAPE_KEY (27)
#define SYSTEM_ERROR (-1)
#define TARGET_CORE 0

#define SOFT_DEADLINE 0.135 // ms

sem_t transformation_semaphore, logging_semaphore;
static int total_deadline_miss = 0;
bool initial = true;

void *(*transformationType)(void *);
std::string resolution;
int schedPolicy = SCHED_FIFO; // Default policy
std::string transformationStr;
volatile int soft_exit = 0;
double sum_of_execution = 0;

std::string filePath = "./example.csv";

struct timespec update_interval;
struct timespec last_update_interval;
struct timespec start_interval;
struct timespec end_interval;

namespace canny
{
    int lowThreshold = 0;
    const int max_lowThreshold = 100;
    const int ratio = 3;
    const int kernel_size = 3;
    const char *window_name = "Edge Map";
}

typedef struct
{
    int threadId;
    int width;
    int height;
    cv::Mat frame;
    cv::Mat intermediate_src;
    cv::Mat output;
    std::string window_name;

} ThreadArgs_t;

typedef struct
{
    int period;
    int burst_time;
    struct sched_param priority_param;
    void *(*thread_handle)(void *);
    pthread_t thread;
    ThreadArgs_t thread_args;
    void *return_Value;
    pthread_attr_t attribute;
    int target_cpu;
} RmTask_t;

// Mat frame;
void print_scheduler(void)
{
    int schedType;
    schedType = sched_getscheduler(getpid());
    switch (schedType)
    {
    case SCHED_FIFO:
        printf("Pthread Policy is SCHED_FIFO\n");
        break;
    case SCHED_OTHER:
        printf("Pthread Policy is SCHED_OTHER\n");
        break;
    case SCHED_RR:
        printf("Pthread Policy is SCHED_OTHER\n");
        break;
    default:
        printf("Pthread Policy is UNKNOWN\n");
    }
}

void appendDataToCsv(int max_frame, float total_time)
{
    // Open the file in append mode
    std::ofstream file(filePath, std::ios::app);

    if (!file.is_open())
    {
        std::cerr << "Failed to open the file for appending." << std::endl;
        return;
    }

    // Write the new data to the file
    file << MAX_FRAME - max_frame << "," << total_time << "," << (1 / total_time) << "," << schedPolicy << "," << resolution << "," << transformationStr << std::endl;

    // Close the file
    file.close();
}

void HoughLinePTransform(RmTask_t *data)
{

    std::vector<cv::Vec4i> linesP;                                                           // Will hold the results of the detection
    cv::HoughLinesP(data->thread_args.intermediate_src, linesP, 1, CV_PI / 180, 50, 50, 10); // Runs the actual detection

    // Draw the lines
    for (size_t i = 0; i < linesP.size(); i++)
    {
        cv::Vec4i l = linesP[i];
        cv::line(data->thread_args.output, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 3, cv::LINE_AA);
        cv::line(data->thread_args.frame, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 3, cv::LINE_AA);
    }
    cv::imshow(data->thread_args.window_name, data->thread_args.output);
}

// Function to detect and draw circles in an image using the Hough Transform
void HoughCircleCam(RmTask_t *data)
{

    cvtColor(data->thread_args.frame, data->thread_args.intermediate_src, cv::COLOR_BGR2GRAY);
    cv::medianBlur(data->thread_args.intermediate_src, data->thread_args.intermediate_src, 5);
    std::vector<cv::Vec3f> circles;

    cv::HoughCircles(data->thread_args.intermediate_src, circles, cv::HOUGH_GRADIENT, 1,
                     data->thread_args.intermediate_src.rows / 16, // Change this value to detect circles with different distances to each other
                     100, 30, 1, 30                                // Change the last two parameters (min_radius & max_radius) to detect larger circles
    );

    for (size_t i = 0; i < circles.size(); i++)
    {
        cv::Vec3i c = circles[i];
        cv::Point center = cv::Point(c[0], c[1]); // Circle center
        circle(data->thread_args.frame, center, 1, cv::Scalar(0, 100, 100), 3, cv::LINE_AA);
        int radius = c[2];
        circle(data->thread_args.frame, center, radius, cv::Scalar(255, 0, 255), 3, cv::LINE_AA);
    }

    cv::imshow("detected circles", data->thread_args.frame);
}

void CannyThreshold(RmTask_t *data)
{
    cvtColor(data->thread_args.frame, data->thread_args.intermediate_src, cv::COLOR_BGR2GRAY);

    /// Reduce noise with a kernel 3x3
    cv::blur(data->thread_args.intermediate_src, data->thread_args.output, cv::Size(3, 3));

    /// Canny detector
    cv::Canny(data->thread_args.output, data->thread_args.output, canny::lowThreshold, canny::lowThreshold * canny::ratio, canny::kernel_size);

    /// Using Canny's output as a mask, we display our result
    cv::Mat timg_grad;

    // Initialize `timg_grad` to be the same size and type as the source frame, but filled with zeros
    // Assuming `data->thread_args.frame` is a cv::Mat
    timg_grad = cv::Mat::zeros(data->thread_args.frame.size(), data->thread_args.frame.type());

    // Using Canny's output as a mask, copy the frame to `timg_grad` wherever the mask is non-zero
    data->thread_args.frame.copyTo(timg_grad, data->thread_args.output);

    // Display the result
    cv::imshow(data->thread_args.window_name, timg_grad);
}

void *HoughLineTransform(void *args)
{
    printf("Woring");
    RmTask_t *data = static_cast<RmTask_t *>(args);
    data->thread_args.window_name = "Houghline Transformation";
    cv::VideoCapture cam0(0);
    cv::namedWindow("video_display");
    char winInput;

    if (!cam0.isOpened())
    {
        exit(SYSTEM_ERROR);
    }

    cam0.set(cv::CAP_PROP_FRAME_WIDTH, data->thread_args.width);
    cam0.set(cv::CAP_PROP_FRAME_HEIGHT, data->thread_args.height);

    int max_frame = MAX_FRAME;
    clock_gettime(CLOCK_REALTIME, &start_interval);

    while (max_frame)
    {
        sem_wait(&transformation_semaphore);
        cam0.read(data->thread_args.frame);
        cv::imshow("video_display", data->thread_args.frame);
        cv::Canny(data->thread_args.frame, data->thread_args.intermediate_src, 80, 240, 3);
        cv::cvtColor(data->thread_args.intermediate_src, data->thread_args.output, cv::COLOR_GRAY2BGR);
        cv::namedWindow(data->thread_args.window_name, cv::WINDOW_AUTOSIZE);
        HoughLinePTransform(data);
        if ((winInput = cv::waitKey(10)) == ESCAPE_KEY)
        {
            soft_exit = 1;
            sem_post(&logging_semaphore);
            cv::destroyAllWindows();
            break;
        }
        max_frame--;
        sem_post(&logging_semaphore);
    }

    return nullptr;
}

void *HoughCircleTransform(void *args)
{

    RmTask_t *data = static_cast<RmTask_t *>(args);
    data->thread_args.window_name = "HoughCircle Transformation";
    cv::VideoCapture cam0(0);
    cv::namedWindow("video_display");
    char winInput;
    if (!cam0.isOpened())
    {
        exit(SYSTEM_ERROR);
    }
    cam0.set(cv::CAP_PROP_FRAME_WIDTH, data->thread_args.width);
    cam0.set(cv::CAP_PROP_FRAME_HEIGHT, data->thread_args.height);

    int max_frame = MAX_FRAME;
    clock_gettime(CLOCK_REALTIME, &start_interval);
    while (max_frame)
    {
        sem_wait(&transformation_semaphore);
        cam0.read(data->thread_args.frame);
        cv::imshow("video_display", data->thread_args.frame);
        HoughCircleCam(data);
        if ((winInput = cv::waitKey(10)) == ESCAPE_KEY)
        {
            soft_exit = 1;
            sem_post(&logging_semaphore);
            cv::destroyAllWindows();
            break;
        }
        else if (winInput == 'n')
        {
            printf("input %c is ignored\n", winInput);
        }
        max_frame--;
        sem_post(&logging_semaphore);
    }

    return nullptr;
}

void *CannyTransform(void *args)
{

    RmTask_t *data = static_cast<RmTask_t *>(args);
    data->thread_args.window_name = "Canny Transformation";
    cv::VideoCapture cam0(0);
    cv::namedWindow("video_display");
    char winInput;

    if (!cam0.isOpened())
    {
        exit(SYSTEM_ERROR);
    }

    cam0.set(cv::CAP_PROP_FRAME_WIDTH, data->thread_args.width);
    cam0.set(cv::CAP_PROP_FRAME_HEIGHT, data->thread_args.height);

    int max_frame = MAX_FRAME;

    clock_gettime(CLOCK_REALTIME, &start_interval);
    while (max_frame)
    {
        sem_wait(&transformation_semaphore);

        cam0.read(data->thread_args.frame);

        cv::imshow("Canny Video", data->thread_args.frame);

        cv::namedWindow(data->thread_args.window_name, cv::WINDOW_AUTOSIZE);

        CannyThreshold(data);

        if ((winInput = cv::waitKey(10)) == ESCAPE_KEY)
        {
            soft_exit = 1;
            sem_post(&logging_semaphore);
            cv::destroyAllWindows();
            break;
        }

        max_frame--;
        sem_post(&logging_semaphore);
    }

    return nullptr;
}

void *LoggingThread(void *args)
{
    int max_frame = MAX_FRAME;
    clock_gettime(CLOCK_REALTIME, &last_update_interval);
    while (max_frame)
    {
        printf("working log");

        sem_wait(&logging_semaphore);

        clock_gettime(CLOCK_REALTIME, &update_interval);
        float total_sec = update_interval.tv_sec - last_update_interval.tv_sec;
        float total_ns = ((float)(update_interval.tv_nsec - last_update_interval.tv_nsec) / 1000000000);
        float total_time = total_sec + total_ns;

        if (!initial)
        {

            if (total_time > SOFT_DEADLINE)
            {
                total_deadline_miss++;
            }
            sum_of_execution += total_time;

            printf("current fps/jitter: update_interval - last_update_interval : %f , total_time taken for 1 frame: %f \n\r", (1 / total_time), total_time);

            appendDataToCsv(max_frame, total_time);
            
        }

        if (soft_exit)
        {
            printf("Exiting thread 2\n\r");
            break;
        }
        sem_post(&transformation_semaphore);
        last_update_interval = update_interval;
        max_frame--;
        initial = false;
    }
    return nullptr;
}

int main(int argc, char **argv)
{

    sem_init(&transformation_semaphore, false, 0);
    sem_init(&logging_semaphore, false, 0);

    sem_post(&transformation_semaphore);

    const std::string expectedHeader = "Number,Execution_time,Frame_rate,Sched_Policy,Resolution,Transform";
    std::ifstream file(filePath);
    std::string currentHeader;

    if (file.is_open())
    {
        // Get the first line from the file
        std::getline(file, currentHeader);
        file.close();

        // Check if the current header matches the expected header
        if (currentHeader != expectedHeader)
        {
            // The headers do not match, so we need to write the correct header

            // Store the rest of the file content
            std::string restOfFileContent;
            std::string line;
            while (std::getline(file, line))
            {
                restOfFileContent += line + "\n";
            }

            // Write the correct header and the rest of the file
            std::ofstream outFile(filePath);
            outFile << expectedHeader << "\n"
                    << restOfFileContent;
            outFile.close();

            std::cout << "Header was corrected in the CSV file." << std::endl;
        }
        else
        {

            std::cout << "CSV header is correct." << std::endl;
        }
    }
    else
    {
        std::cerr << "Could not open the file." << std::endl;
        return 1;
    }

    void *(*transformationType)(void *) = CannyTransform;
    std::map<std::string, int> schedPolicyMap = {
        {"SCHED_FIFO", SCHED_FIFO},
        {"SCHED_RR", SCHED_RR},
        {"SCHED_OTHER", SCHED_OTHER}};

    std::map<std::string, void *(*)(void *)> TransformationPolicyMap = {
        {"Canny", CannyTransform},
        {"HoughLine", HoughLineTransform},
        {"HoughCircle", HoughCircleTransform}};

    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg.find("--transformation=") == 0)
        {
            transformationStr = arg.substr(std::string("--transformation=").length());
            if (TransformationPolicyMap.find(transformationStr) != TransformationPolicyMap.end())
            {
                transformationType = TransformationPolicyMap[transformationStr];
            }
            else
            {
                std::cerr << "Unknown transformation type: " << transformationStr << std::endl;
                std::exit(EXIT_FAILURE);
            }
        }
        else if (arg.find("--resolution=") == 0)
        {
            resolution = arg.substr(std::string("--resolution=").length());
        }
        else if (arg.find("--sched_policy=") == 0)
        {
            std::string policyStr = arg.substr(std::string("--sched_policy=").length());
            if (schedPolicyMap.find(policyStr) != schedPolicyMap.end())
            {
                schedPolicy = schedPolicyMap[policyStr];
            }
            else
            {
                std::cerr << "Unknown scheduler policy: " << policyStr << std::endl;
                std::exit(EXIT_FAILURE);
            }
        }
    }

    std::cout << "Resolution: " << resolution << std::endl;
    std::cout << "Scheduler Policy: " << schedPolicy << std::endl;

    pthread_t threads[NUM_OF_THREAD];
    cpu_set_t threadcpu;

    CPU_SET(TARGET_CORE, &threadcpu);

    RmTask_t tasks[NUM_OF_THREAD] = {
        {.period = 20,     // ms
         .burst_time = 10, // ms
         .priority_param = {0},
         .thread_handle = transformationType,
         .thread = threads[0],
         .thread_args = {0, 0, 0},
         .return_Value = NULL,
         .attribute = {0, 0},
         .target_cpu = TARGET_CORE},

        {.period = 20,     // ms
         .burst_time = 10, // ms
         .priority_param = {0},
         .thread_handle = LoggingThread,
         .thread = threads[0],
         .thread_args = {0, 0, 0},
         .return_Value = NULL,
         .attribute = {0, 0},
         .target_cpu = TARGET_CORE},
    };

    pthread_attr_t attribute_flags_for_main; // for schedular type, priority
    struct sched_param main_priority_param;

    printf("This system has %d processors configured and %d processors available.\n", get_nprocs_conf(), get_nprocs());

    printf("Before adjustments to scheduling policy:\n");
    print_scheduler();

    int rt_max_prio = sched_get_priority_max(schedPolicy);
    int rt_min_prio = sched_get_priority_min(schedPolicy);

    main_priority_param.sched_priority = rt_max_prio;
    for (int i = 0; i < NUM_OF_THREAD; i++)
    {
        tasks[i].priority_param.sched_priority = rt_max_prio;

        // initialize attributes
        pthread_attr_init(&tasks[i].attribute);

        pthread_attr_setinheritsched(&tasks[i].attribute, PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setschedpolicy(&tasks[i].attribute, schedPolicy);
        pthread_attr_setschedparam(&tasks[i].attribute, &tasks[i].priority_param);
        pthread_attr_setaffinity_np(&tasks[i].attribute, sizeof(cpu_set_t), &threadcpu);
    }

    pthread_attr_init(&attribute_flags_for_main);

    // pthread_attr_setinheritsched(&attribute_flags_for_main, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attribute_flags_for_main, schedPolicy);
    pthread_attr_setaffinity_np(&attribute_flags_for_main, sizeof(cpu_set_t), &threadcpu);

    // Main thread is already created we have to modify the priority and scheduling scheme
    int status_setting_schedular = sched_setscheduler(getpid(), schedPolicy, &main_priority_param);
    if (status_setting_schedular)
    {
        printf("ERROR; sched_setscheduler rc is %d\n", status_setting_schedular);
        perror(NULL);
        exit(-1);
    }

    printf("After adjustments to scheduling policy:\n");
    print_scheduler();

    int width = 0, height = 0;
    size_t xPosition = resolution.find('x');
    if (xPosition != std::string::npos)
    {
        width = std::stoi(resolution.substr(0, xPosition));
        height = std::stoi(resolution.substr(xPosition + 1));
    }

    for (int i = 0; i < NUM_OF_THREAD; i++)
    {
        // Create a thread
        // First paramter is thread which we want to create
        // Second parameter is the flags that we want to give it to
        // third parameter is the routine we want to give
        // Fourth parameter is the value
        printf("Setting thread %d to core %d\n", i, TARGET_CORE);

        if (pthread_create(&tasks[i].thread, &tasks[i].attribute, tasks[i].thread_handle, &tasks[i]) != 0)
        {
            perror("Create_Fail");
        }
    }
    printf("Threads created \n\r");
    for (int i = 0; i < NUM_OF_THREAD; i++)
    {
        printf("Joining thread %d\n", i);
        int join_result = pthread_join(tasks[i].thread, &tasks[i].return_Value);
        if (join_result != 0)
        {
            printf("pthread_join failed for thread %d: %s\n", i, strerror(join_result));
        }
        else
        {
            printf("Thread %d joined successfully.\n", i);
        }
    }

    if (pthread_attr_destroy(&tasks[0].attribute) != 0)
        perror("attr destroy");
    if (pthread_attr_destroy(&tasks[1].attribute) != 0)
        perror("attr destroy");

    clock_gettime(CLOCK_REALTIME, &end_interval);

    float total_sec = end_interval.tv_sec - start_interval.tv_sec;
    float total_ns = ((float)(end_interval.tv_nsec - start_interval.tv_nsec) / 1000000000.0);
    float total_time = (total_sec + total_ns);

    printf("Average FPS %f, Average time %f \n\r", (MAX_FRAME / sum_of_execution), sum_of_execution/MAX_FRAME);
    printf("Total deadline miss : %d\n\r", total_deadline_miss);
    sem_destroy(&transformation_semaphore);
    sem_destroy(&logging_semaphore);

    return 0;
}