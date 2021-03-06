/** 
 * \file vc0706_device.c
 * \brief Contains additional VC0706 communication logic (see vc0706_core.c)
 */
#include "vc0706.h"
#include "vc0706_child.h"
#include "vc0706_device.h"

/** Parallel Pins */
int PARALLEL_PIN_BUS[6] = {36, 35, 34, 33, 32, 31};

// External References
extern vc0706_hk_tlm_t VC0706_HkTelemetryPkt;
extern struct led_t led; /**< LED instance from vc0706.c */
extern struct Camera_t cam;

/** Holds the number of times the system has rebooted (populated by VC0706_setNumReboots()) */
char num_reboots[3];

/**
 * Core loop for taking pictures
 */
int VC0706_takePics(void)
{
    int32 hk_packet_succes = 0;

    /*
    ** Path that pictures should be stored in
    **
    ** NOTE: if path is greater than 16 chars, imageName[] in vc0706_core.h will need to be enlarged accordingly.
    */
    char path[OS_MAX_PATH_LEN];
    memset(path, '\0', sizeof(path));

    char file_name[15];
    memset(file_name, '\0', sizeof(file_name));

    /*
    ** get Num reboots
    */
    //OS_printf("VC0706: Attempting to VC0706_setNumReboots()...\n");
    //VC0706_setNumReboots();

    /*
    ** Attempt to initialize LED
    */
    led_init(&led, (int)LED_PIN);

    /*
    ** Attempt to initalize Camera #1
    */
    if (init(&cam, 0) == -1) // Error
    {
        OS_printf("Camera initialization error.\n");
        return -1;
    }

    /*
    ** Initialize the Parallel Pins
    */
    setupParallelPhotoCount();

    /*
    ** infinite Camera loop
    ** w/ no delay
    */
    unsigned int num_pics_stored = 1;
    for (;;)
    {

        /*
        ** Get camera version, another way to check that the camera is working properly. Also necessary for initialization.
        **
        ** NOTE: Not sure if this should be done every loop iteration. It is a good way to check on the Camera, but maybe wasteful of time.
        */
        if ((getVersion(&cam)) == -1)
        {
            OS_printf("Failed communication to Camera.\n"); // NOTE: vc0706_core::checkReply() does CVE logging.
            // return -1; // should never stop the task, just keep trying.
            continue; // loop start over
        }

        /*
        ** Set Path for the new image
	    **
	    ** Format:
	    ** /ram/images/<num_reboots>_<camera 0 or 1>_<num_pics_stored>.jpg
        */
        //OS_printf("VC0706: Calling sprintf()...\n");

        int ret = 0;
        ret = snprintf(file_name, sizeof(file_name), "%.3s_%d_%.4u.jpg", num_reboots, cam.ttyInterface, num_pics_stored); // cFS /exe relative path
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

        /*
        ** Actually take the picture
        */
        //OS_printf("VC0706: Calling takePicture(&cam, \"%s\")...\n", path);
        char *pic_file_name = takePicture(&cam, path);
        if (pic_file_name != (char *)NULL)
        {
            //OS_printf("Debug: Camera took picture. Stored at: %s\n", pic_file_name);

            /*
		    ** Put Image name on telem packet
		    */
            if ((hk_packet_succes = snprintf(VC0706_HkTelemetryPkt.vc0706_filename, 15, "%.*s", 15, (char *)pic_file_name + 12)) < 0) // only use the filename, not path.
            {
                OS_printf("VC0706: ERROR: HK sprintf ret [%d] filename [%.*s]\n", (int)hk_packet_succes, 15, (char *)&pic_file_name[12]);
                // continue
            }
            else
            {
                //OS_printf("VC0706: Wrote Picture Filename to HK Packet. Sent: '%.*s'\n", 15, (char * )&pic_file_name[12], hk_packet_succes);
                VC0706_SendTimFileName(file_name);
            }
            //OS_printf("VC0706: VC0706_HkTelemetryPkt.vc0706_filename: '%s'\n", VC0706_HkTelemetryPkt.vc0706_filename);

            /*
			** update number of pics taken on the parallel pins
			*/
            updatePhotoCount((uint8)num_pics_stored);

            /*
		    ** incriment num pics for filename
		    */
            num_pics_stored++;
        }
        else
        {
            VC0706_SendTimFileName("error.txt"); // contains: "image failed to be taken."
        }

    } /* Infinite Camera capture Loop End Here */

    return (0);
}

/**
 * initializes parallel pins on the parallel line designated for photo count [pins 31-36]
 */
void setupParallelPhotoCount(void)
{
    int i;
    for (i = 0; i < 6; i++)
    {
        pinMode(PARALLEL_PIN_BUS[i], OUTPUT);
    }

    // initialize to 0
    updatePhotoCount((uint8)0);
}

/**
 * Writes the specified int to 6 bits on out parallel line.
 * This function assumes the designated ouputs are already initialized.
 */
void updatePhotoCount(uint8 pic_count)
{
    int gpio_pin;
    int i;
    for (i = 0; i < 6; i++)
    {
        gpio_pin = PARALLEL_PIN_BUS[i];

        if (pic_count & (1 << i))
        {
            digitalWrite(gpio_pin, HIGH);
        }
        else
        {
            digitalWrite(gpio_pin, LOW);
        }
    }
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