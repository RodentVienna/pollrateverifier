
struct input_event;

class PollrateVerifier
{
  public:
    void run();
  private:
    void quit( const char* msg );
    void keyPress( bool pressed, int fd, input_event& ev );
};

