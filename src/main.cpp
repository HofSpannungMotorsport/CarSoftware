#include "mbed.h"

#include "car/Car.h"
#include <memory>
#include "RuntimeContext.h"


int main()
{
    shared_ptr<RuntimeContext> context = make_shared<RuntimeContext>(RuntimeContext());
    initBoardHardware(context);

    while(1)
    {
        context->executeAllComponents();
        wait_us(500);
    }
    return 0;
}
