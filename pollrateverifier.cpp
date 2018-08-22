
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <time.h>

#include <iostream>

#include "pollrateverifier.h"

void PollrateVerifier::quit( const char* msg)
{
  perror(msg);
  exit(1);
}

void PollrateVerifier::keyPress( bool pressed, int fd, input_event& ev )
{
  // Register the key press event
  memset(&ev, 0, sizeof(input_event));
  ev.type = EV_KEY;
  ev.code = KEY_A;
  ev.value = pressed ? 1 : 0;
  if( write(fd, &ev, sizeof(input_event)) < 0 )
  {
    quit("Failed to press key");
  }

  // Register the key release event
  memset(&ev, 0, sizeof(input_event));
  ev.type = EV_SYN;
  ev.code = SYN_REPORT;
  ev.value = 0;
  if( write(fd, &ev, sizeof(input_event)) < 0 )
  {
    quit("Failed to release key");
  }
}

void PollrateVerifier::run()
{
  int fd;
  uinput_user_dev uidev;
  input_event ev;

  // Open the uinput driver
  fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  if( fd < 0 )
  {
    quit("Failed to open /dev/uinput");
  }

  // Setup our fake input device
  memset(&uidev, 0, sizeof(uidev));
  snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "uinput-spammer-test");
  uidev.id.bustype = BUS_USB;
  // These IDs identify the provider and device attached to the USB bus
  // In this case just use 0x01. I couldn't find any accepted 'example' value
  uidev.id.vendor  = 0x01;
  uidev.id.product = 0x01;
  uidev.id.version = 1;

  // Register what our input device can do
  if( ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0 )
  {
    quit("ioctl");
  }
  if( ioctl(fd, UI_SET_KEYBIT, KEY_A) < 0 )
  {
    quit("ioctl");
  }

  // And create the device
  if( write(fd, &uidev, sizeof(uidev)) < 0 )
  {
    quit("Failed to write to uidev");
  }
  if( ioctl(fd, UI_DEV_CREATE) < 0 )
  {
    quit("Failed to create uinput device");
  }

  // TODO: Right now let's have a short delay and let the user start evhz in another terminal. Really we want a background thread started to trigger input and a foreground thread to report on the results
  for( auto i = 10u; i > 0; --i )
  {
    std::cout << "Starting input in " << i << " seconds" << std::endl;
    sleep(1);
  }

  std::cout << "Starting input. Use Ctrl+C to stop" << std::endl;

  while( true ) 
  {
    // Trigger keydown/up with a small wait in between
    // We're interested in polling rates up to 1Khz so
    // spam events at twice this rate
    keyPress( true, fd, ev );
    usleep(500);
    keyPress( false, fd, ev );
    usleep(500);
  }

  // Cleanup and exit
  if( ioctl(fd, UI_DEV_DESTROY) < 0 )
  {
    quit( "Failed to destroy input device" );
  }

  close(fd);
}
