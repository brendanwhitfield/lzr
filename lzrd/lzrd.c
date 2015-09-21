
/*
    Right now, this is mostly just glue that puts an
    etherdream on a ZMQ subscriber socket.
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>

#include <lzr.h>
#include <zmq.h>
#include "libetherdream/etherdream.h"


#define CLAMP(d) ( fmin(fmax(d, -1.0), 1.0) )

typedef struct etherdream etherdream;
typedef struct etherdream_point etherdream_point;


static void*             zmq_ctx;
static void*             rx;
static lzr_frame*        frame;
static etherdream*       dac;
static etherdream_point* ether_points;


static void send_frame()
{
    //if the laser is ready for another frame
    if(etherdream_is_ready(dac) == 1)
    {
        printf("RECV frame (%d points)\n", frame->n_points);

        for(size_t i = 0; i < frame->n_points; i++)
        {
            //convert LZR point into etherdream point
            ether_points[i].x = (int16_t) (CLAMP(frame->points[i].x) * 32767);
            ether_points[i].y = (int16_t) (CLAMP(frame->points[i].y) * 32767);
            ether_points[i].r = (uint16_t) (frame->points[i].r * 255);
            ether_points[i].g = (uint16_t) (frame->points[i].g * 255);
            ether_points[i].b = (uint16_t) (frame->points[i].b * 255);
            ether_points[i].i = (uint16_t) (frame->points[i].i * 255);
        }

        etherdream_write(dac, ether_points, frame->n_points, 30000, -1);
    }
    //else, dump the frame, an old one is still being drawn
    //TODO: ^ is this really a good idea? Could create a stutterring animation
}


//main laser client
int main()
{
    int rc       = 0;
    zmq_ctx      = zmq_ctx_new();
    rx           = lzr_create_frame_rx(zmq_ctx, LZR_ZMQ_ENDPOINT);
    frame        = (lzr_frame*) malloc(sizeof(lzr_frame));
    ether_points = (etherdream_point*) calloc(sizeof(etherdream_point), LZR_FRAME_MAX_POINTS);

    etherdream_lib_start();

    //discover DACs
    int dac_count = 0;
    while(dac_count == 0)
    {
        printf("Searching for Etherdream...\n");
        sleep(1);
        dac_count = etherdream_dac_count();
    }

    printf("Found %d Etherdream(s)...\n", dac_count);
    printf("Connecting to Etherdream...\n");

    dac = etherdream_get(0);
    rc = etherdream_connect(dac);
    assert(rc == 0);

    printf("Connection successful...\n");

    //enter the main loop
    //-------------------
    while(1)
    {
        lzr_recv_frame(rx, frame);
        send_frame();
    }
    //-------------------

    etherdream_stop(dac);
    etherdream_disconnect(dac);

    free(ether_points);
    free(frame);
    zmq_close(rx);
    zmq_ctx_term(zmq_ctx);

    return rc;
}
