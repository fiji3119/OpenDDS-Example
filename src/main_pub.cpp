/*
 * main_pub.cpp
 *
 *  Created on: Nov 9, 2020
 *      Author: nkumar
 */

#include <cstdint>
#include <chrono>
#include <thread>
#include <csignal>
#include "Subscription.h"
#include "Publication.h"
#include "ImageTypeSupportImpl.h"

using namespace MyExample;

const int32_t WIDTH = 640;
const int32_t HEIGHT = 480;
const float   FRAME_RATE = 30.0f;
const int32_t FRAME_INTERVAL = static_cast<int32_t>((1.0/FRAME_RATE)*1000000); //microsec


int32_t gAbort = 0;
void signal_handler(int32_t signal)
{
    if (signal == SIGINT) gAbort = 1;
}

void initial_frame(uint16_t *pixels)
{
    if (pixels)
    {
        const int32_t colorBarWidth = WIDTH/7; /* 7 Colors in test pattern */
        const int32_t white_start   = 0;
        const int32_t white_end     = colorBarWidth;
        const int32_t yellow_start  = white_end;
        const int32_t yellow_end    = yellow_start + colorBarWidth;
        const int32_t cyan_start    = yellow_end;
        const int32_t cyan_end      = cyan_start + colorBarWidth;
        const int32_t green_start   = cyan_end;
        const int32_t green_end     = green_start + colorBarWidth;
        const int32_t magenta_start = green_end;
        const int32_t magenta_end   = magenta_start + colorBarWidth;
        const int32_t red_start     = magenta_end;
        const int32_t red_end       = red_start + colorBarWidth;
        const int32_t blue_start    = red_end;

        //Start with 'gray'
        uint8_t Y=180, U=128, V=128;
        int32_t idx = 0;

        for (int32_t y = 0; y < HEIGHT; ++y)
        {
            for (int32_t x = 0; x < WIDTH; ++x)
            {
                if (x > white_start && x < white_end)
                {
                    Y = 235; U = 128; V = 128;
                }
                if (x > yellow_start && x < yellow_end)
                {
                    Y = 168; U = 44; V = 136;
                }
                if (x > cyan_start && x < cyan_end)
                {
                    Y = 145; U = 147; V = 44;
                }
                if (x > green_start && x < green_end)
                {
                    Y = 134; U = 63; V = 52;
                }
                if (x > magenta_start && x < magenta_end)
                {
                    Y = 63; U = 193; V = 204;
                }
                if (x > red_start && x < red_end)
                {
                    Y = 51; U = 109; V = 212;
                }
                if (x > blue_start && x < WIDTH)
                {
                    Y = 28; U = 212; V = 120;
                }

                if( (idx%2) )  pixels[idx] = (Y << 8) | U;
                else           pixels[idx] = (Y << 8) | V;

                idx++;
            }//endfor x
        }//endfor y

    }//endif
}

int32_t main(int32_t argc, char *argv[])
{
    signal(SIGINT, signal_handler);

    // Initialize DomainParticipantFactory
    DDS::DomainParticipantFactory_var dpf = TheParticipantFactoryWithArgs(argc, argv);

    std::string topic_images("images");
    Publication<msg::Image> *pub = new Publication<msg::Image>(dpf, topic_images);

    //Enable publisher
    DDS::ReturnCode_t ret;
    if ( (ret = pub->enable()) != DDS::RETCODE_OK )
    {
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%N:%l: Publisher enable failed! (%d)\n"), ret), -1);
    }

    //Create YUV422
    const int32_t depth = 2; //Y, (U/V)
    msg::Image msg;
    ORBSVCS_Time::hrtime_to_TimeT(msg.timestamp, ACE_OS::gethrtime());
    msg.width = WIDTH;
    msg.height = HEIGHT;
    msg.framecount = 0;
    msg.data.length( /*Y*/WIDTH*HEIGHT*depth /*U,V*/ );

    //Init frame
    uint16_t *pixel_ptr = reinterpret_cast<uint16_t*>(msg.data.get_buffer());
    initial_frame( pixel_ptr );

    //Picture-in-picture box size
    const int32_t pip_width = WIDTH/4;
    const int32_t pip_height = pip_width;
    const int32_t pip_left_x = WIDTH-pip_width-10;
    const int32_t pip_left_y = HEIGHT-pip_height-10;

    //Update frames and publish
    ACE_Time_Value interval(0, FRAME_INTERVAL);
    while (!gAbort)
    {
        //Create random noise in picture-in-picture
        for (int32_t y = pip_left_y; y < (pip_left_y + pip_height); ++y)
        {
            for (int32_t x = pip_left_x; x < (pip_left_x + pip_width); ++x)
            {
                uint8_t Y = rand() % 255;
                pixel_ptr[y*WIDTH + x] = (Y << 8) | 128;
            }
        }
        pub->publish(msg);
        msg.framecount++;
        ACE_OS::sleep(interval);
    }

    pub->get_participant()->delete_contained_entities();
    dpf->delete_participant(pub->get_participant());
    delete pub;
    return 0;
}
