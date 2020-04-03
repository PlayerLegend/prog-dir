#include "vulkan_testing.h"

#include <unistd.h>

int main()
{
    vulkan_state state;
    
    init_vulkan(&state);

    state.window = create_window(640,480,"test window");

    //while(!vulkan_should_exit(&state))
	sleep(1);

    destroy_window(state.window);
    
    halt_vulkan(&state);

    return 0;
}
