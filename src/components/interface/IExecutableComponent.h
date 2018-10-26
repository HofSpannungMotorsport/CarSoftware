#ifndef IEXECUTEABLE_COMPONENT_H
#define IEXECUTEABLE_COMPONENT_H

enum execution_result_t
{
    OK = 1,
    ERROR = -1
};

class IExecuteableComponent
{   
    public:
        virtual execution_result_t execute() = 0;
};

#endif