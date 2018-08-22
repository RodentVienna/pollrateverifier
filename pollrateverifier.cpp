
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

PollrateVerifier::PollrateVerifier()
{
  // Open the uinput driver
  m_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  if( m_fd < 0 )
  {
    quit("Failed to open /dm_ev/uinput");
  }

  // Setup our fake input dm_evice
  memset(&m_uidev, 0, sizeof(m_uidev));
  snprintf(m_uidev.name, UINPUT_MAX_NAME_SIZE, "uinput-spammer-test");
  m_uidev.id.bustype = BUS_USB;
  // These IDs identify the provider and dm_evice attached to the USB bus
  // In this case just use 0x01. I couldn't find any accepted 'example' value
  m_uidev.id.vendor  = 0x01;
  m_uidev.id.product = 0x01;
  m_uidev.id.version = 1;

  // Register what our input dm_evice can do
  if( ioctl(m_fd, UI_SET_EVBIT, EV_KEY) < 0 )
  {
    quit("ioctl");
  }
  if( ioctl(m_fd, UI_SET_KEYBIT, KEY_A) < 0 )
  {
    quit("ioctl");
  }

  // And create the dm_evice
  if( write(m_fd, &m_uidev, sizeof(m_uidev)) < 0 )
  {
    quit("Failed to write to m_uidev");
  }
  if( ioctl(m_fd, UI_DEV_CREATE) < 0 )
  {
    quit("Failed to create uinput dm_evice");
  }

}

PollrateVerifier::~PollrateVerifier()
{
  // Cleanup and exit
  if( ioctl(m_fd, UI_DEV_DESTROY) < 0 )
  {
    quit( "Failed to destroy input dm_evice" );
  }

  close(m_fd);
}

void PollrateVerifier::quit( const char* msg)
{
  perror(msg);
  exit(1);
}

void PollrateVerifier::keyPress( int key, bool pressed )
{
  // Register the key press m_event
  memset(&m_ev, 0, sizeof(input_event));
  m_ev.type = EV_KEY;
  m_ev.code = KEY_A;
  m_ev.value = pressed ? 1 : 0;
  if( write(m_fd, &m_ev, sizeof(input_event)) < 0 )
  {
    quit("Failed to press key");
  }

  // Register the key release m_event
  memset(&m_ev, 0, sizeof(input_event));
  m_ev.type = EV_SYN;
  m_ev.code = SYN_REPORT;
  m_ev.value = 0;
  if( write(m_fd, &m_ev, sizeof(input_event)) < 0 )
  {
    quit("Failed to release key");
  }
}

void PollrateVerifier::run()
{

  // TODO: Right now let's have a short delay and let the user start m_evhz in another terminal. Really we want a background thread started to trigger input and a foreground thread to report on the results
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
    // spam m_events at twice this rate
    keyPress( KEY_A, true );
    usleep(500);
    keyPress( KEY_A, false );
    usleep(500);
  }

}
