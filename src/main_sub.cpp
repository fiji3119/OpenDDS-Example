/*
 * main_sub.cpp
 *
 *  Created on: Nov 30, 2020
 *      Author: nkumar
 */
#include <cstdint>
#include <csignal>
#include <opencv/cv.hpp>
#include "Subscription.h"
#include "Publication.h"
#include "ImageTypeSupportImpl.h"

using namespace MyExample;

int32_t gAbort = 0;
void signal_handler(int32_t signal)
{
    gAbort = 1;
}


void images_callback(const msg::Image& msg)
{
    uint8_t *pixels = new uint8_t[msg.width * msg.height * 2];
    memcpy(pixels, msg.data.get_buffer(), msg.width*msg.height*2);
    cv::Mat yuv422 = cv::Mat(cv::Size(msg.width, msg.height), CV_8UC2, pixels);
    cv::Mat rgba;
    cv::cvtColor(yuv422, rgba, CV_YUV2RGBA_Y422);
    cv::String text_str = cv::String("MyExample - YUV422");
    int32_t baseline = 0;
    cv::Size text_str_size = cv::getTextSize(text_str, cv::FONT_HERSHEY_SIMPLEX, 1.0, 1, &baseline);

    //Put some text
    cv::putText(rgba,
                text_str,
                cv::Point(msg.width/2-(text_str_size.width/2), msg.height/2),
                cv::FONT_HERSHEY_PLAIN,
                1.0,
                cv::Scalar(255),
                1
               );
     //Put frame count
    text_str = std::string("Frame Count: " + std::to_string(msg.framecount));
    cv::putText(rgba,
                text_str,
                cv::Point(5,10),
                cv::FONT_HERSHEY_PLAIN,
                1.0,
                cv::Scalar(255),
                1
               );
    //Display
    cv::imshow("View", rgba);
    cv::waitKey(5);
    delete [] pixels;
}

int32_t main(int32_t argc, char *argv[])
{
    signal(SIGINT, signal_handler);

    // Initialize DomainParticipantFactory
    DDS::DomainParticipantFactory_var dpf = TheParticipantFactoryWithArgs(argc, argv);

    std::string topic_images("images");
    Subscription<msg::Image> *sub = new Subscription<msg::Image>(dpf,
                                                                 topic_images,
                                                                 images_callback);
     //Enable subscriber
    DDS::ReturnCode_t ret;
    if ( (ret = sub->enable()) != DDS::RETCODE_OK )
    {
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%N:%l: Subscription enable failed! (%d)\n"), ret), -1);
    }


    ACE_Time_Value interval(1,0); //1 sec
    while (!gAbort)
    {
        ACE_OS::sleep(interval);
    }
    sub->get_participant()->delete_contained_entities();
    dpf->delete_participant(sub->get_participant().in());
    delete sub;
    return 0;
}

