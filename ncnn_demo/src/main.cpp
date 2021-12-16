/*
 * Copyright (C) 2021 XRADIO TECHNOLOGY CO., LTD. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the
 *       distribution.
 *    3. Neither the name of XRADIO TECHNOLOGY CO., LTD. nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include "ohos_init.h"
#include "kernel/os/os.h"

#include "ncnn/benchmark.h"
#include "ncnn/cpu.h"
#include "ncnn/net.h"
#include "ncnn/mat.h"
#include "ncnn/datareader.h"

#include "mnist-1-bin.h"
#include "fs.h"

static OS_Thread_t g_main_thread;

#ifdef __cplusplus
extern "C" {
#endif

void app_main(void)
{
    ncnn::Net net;

    printf("Loading ncnn mnist model...");
    net.load_param(mnist_1_opt_param_bin);
    net.load_model(mnist_1_opt_bin);
//     net.opt.lightmode = true;
    printf("Done.\n");

//     return;

    printf("Preparing input...");

    ncnn::Mat in = ncnn::Mat::from_pixels(IN_IMG, ncnn::Mat::PIXEL_GRAY, IMAGE_W, IMAGE_H);
    ncnn::Mat out;

    printf("[28, 28, 1].\n");

    printf("Start Mesuring!\n");

    double total_latency = 0;
    float max = -1;
    float min = 10000000000000000;

    for(int i=0; i<10; i++){
//         vTaskDelay(20 / portTICK_PERIOD_MS);

//         long start = esp_timer_get_time();
        double start = ncnn::get_current_time();

        ncnn::Extractor ex = net.create_extractor();
        ex.input(0, in);
        ex.extract(17, out);

//         long end = esp_timer_get_time();
        double end = ncnn::get_current_time();

        float lat = end - start;
        total_latency += lat;
        if(lat > max){
            max = lat;
        }
        if(lat < min){
            min = lat;
        }
    }

    printf("Done!\n");

    const float* ptr = out.channel(0);
    int gussed = -1;
    float guss_exp = -10000000;
    for(int i=0; i<out.w * out.h; i++){
        printf("%d: %.2f\n", i, ptr[i]);
        if(guss_exp < ptr[i]){
            gussed = i;
            guss_exp = ptr[i];
        }
    }

    printf("I think it is number %d!\n", gussed);

    printf("Latency, avg: %.2fms, max: %.2f, min: %.2f. Avg Flops: %.2fMFlops\n",total_latency / 10.0, max, min, 0.78 / (total_latency / 10.0 / 1000.0));
}

static void MainThread(void *arg)
{
    app_main();

	while (1) {

        int cpu_count = ncnn::get_cpu_count();
        printf("ncnn::get_cpu_count() = %d\r\n", cpu_count);

        double time = ncnn::get_current_time();
        printf("ncnn::get_current_time() = %f\r\n", time);

        printf("hello world!\r\n");
        extern void LOS_Msleep(unsigned int mSecs);
		LOS_Msleep(1000);
	}
}

void HelloTestMain(void)
{
	printf("Wifi Test Start\n");

	if (OS_ThreadCreate(&g_main_thread, "MainThread", MainThread, NULL,
			    OS_THREAD_PRIO_APP, 4 * 1024) != OS_OK) {
		printf("[ERR] Create MainThread Failed\n");
	}
}

#ifdef __cplusplus
}
#endif

SYS_RUN(HelloTestMain);
