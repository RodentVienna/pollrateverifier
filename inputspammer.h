
#include <linux/input.h>
#include <linux/uinput.h>

class InputSpammer
{
  public:
    InputSpammer();
    ~InputSpammer();
    void run();
  private:
    void quit( const char* msg );
    void keyPress( int key, bool pressed );

    int m_fd = -1;
    input_event m_ev;
    uinput_user_dev m_uidev;
};

