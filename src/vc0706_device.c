/** 
 * \file vc0706_device.c
 * \brief Contains additional VC0706 communication logic (see vc0706_core.c)
 */
#include "vc0706.h"
// TODO: untangle these headers so that this file doesn't need the child task header
#include "vc0706_child.h"
#include "vc0706_device.h"

/** Parallel Pins */
int PARALLEL_PIN_BUS[6] = {36, 35, 34, 33, 32, 31};
/** Represents the LED flash for the camera */
led_t led;

// External References
extern vc0706_hk_tlm_t VC0706_HkTelemetryPkt;

/** Holds the number of times the system has rebooted (populated by VC0706_setNumReboots()) */
char num_reboots[3];

/**
 * Core loop for taking pictures
 * @param[in,out] cameras - An array of camera structs to initialize and use to take pictures
 * @param[in] numberOfCameras - The length of the cameras array
 */
int VC0706_takePics(Camera_t cameras[], int numberOfCameras)
{
    int32 hk_packet_success = 0;

    // Initialize an empty path string
    char path[OS_MAX_PATH_LEN];
    memset(path, '\0', sizeof(path));

    // Initialize an empty filename string
    char file_name[15];
    memset(file_name, '\0', sizeof(file_name));

    // Attempt to initialize LED
    led_init(&led, (int)LED_PIN);

    unsigned int num_pics_stored = 1;
    // infinite Camera loop w/ no delay
    while (true)
    {
        // TODO: probably change this to something like currentCamera for readability
        int i;
        for (i = 0; i < numberOfCameras; i++)
        {
            /*
               Get camera version, another way to check that the camera is working properly. Also necessary for initialization.
              
               NOTE: Not sure if this should be done every loop iteration. It is a good way to check on the Camera, but maybe wasteful of time.
            */
            if (getVersion(&(cameras[i])) == -1)
            {
                // NOTE: vc0706_core::checkReply() (called in getVersion()) does CVE logging.
                OS_printf("Failed communication to Camera %d.\n", i);
                // try the other camera
                continue;
            }

            /*
              Set Path for the new image
            
              Format:
              /ram/images/<num_reboots>_<camera 0 or 1>_<num_pics_stored>.jpg
            */

            int ret = 0;
            ret = snprintf(file_name, sizeof(file_name), "%.3s_%d_%.4u.jpg", num_reboots, i, num_pics_stored); // cFS /exe relative path
            if (ret < 0)
            {
                OS_printf("sprintf err: %s\n", strerror(ret));
                continue;
            }

            ret = snprintf(path, sizeof(path), "/ram/images/%s", file_name); // cFS /exe relative path
            if (ret < 0)
            {
                OS_printf("sprintf err: %s\n", strerror(ret));
                continue;
            }

            // Actually take the picture
            char *pic_file_name = takePicture(&(cameras[i]), path);
            if (pic_file_name != (char *)NULL)
            {
                // Put Image name on telem packet
                if ((hk_packet_success = snprintf(VC0706_HkTelemetryPkt.vc0706_filename, 15, "%.*s", 15, (char *)pic_file_name + 12)) < 0) // only use the filename, not path.
                {
                    OS_printf("VC0706: ERROR: HK sprintf ret [%d] filename [%.*s]\n", (int)hk_packet_success, 15, (char *)&pic_file_name[12]);
                }
                else
                {
                    VC0706_SendTimFileName(file_name);
                }
                // increment num pics for filename
                num_pics_stored++;
            }
            else
            {
                VC0706_SendTimFileName("error.txt"); // contains: "image failed to be taken."
            }
        }
    }
    return (0);
}

/**
 * Reads the number of reboots since mission start (from /ram/logs/reboot.txt file)
 */
void VC0706_setNumReboots(void)
{
    // Reset all of the characters in num_reboots to NULL
    memset(num_reboots, '0', sizeof(num_reboots));

    // Define file permissions for OS_open - this is required in case the file doesn't exist.
    //       Owner write | Owner read | Group read | Group write | Others read
    mode_t mode = S_IWRITE | S_IREAD | S_IRGRP | S_IWGRP | S_IROTH;

    // Open the system's reboot log
    int32 fd = OS_open((const char *)"/ram/logs/reboot.txt", (int32)OS_READ_ONLY, (uint32)mode);

    // Check for file open success
    if (fd != OS_FS_SUCCESS)
        OS_printf("\tCould not open reboot file in VC, ret = %d!\n", (int)fd);

    // Read three characters from file into num_reboots
    int os_ret = OS_read(fd, (void *)num_reboots, 3);
    // Make sure the above read completed successfully
    if (os_ret != OS_FS_SUCCESS)
    {
        // If not, set all num_reboots values to 9
        memset(num_reboots, '9', sizeof(num_reboots));
        OS_printf("\tCould not read from reboot file in VC, ret = %d!\n", os_ret);
    }

    // Close the file handle (for /ram/logs/reboot.txt)
    OS_close(fd);
}