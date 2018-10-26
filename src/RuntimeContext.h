#ifndef RUNTIME_CONTEXT_H
#define RUNTIME_CONTEXT_H

#include <vector>
#include <memory>
#include "components/interface/IExecutableComponent.h"

using namespace std;

class RuntimeContext
{
    private:
        vector<shared_ptr<IExecuteableComponent>> _components;


    public:
        void addComponent(shared_ptr<IExecuteableComponent> component)
        {
            _components.emplace_back(component);
        }
        void executeAllComponents()
        {
            for(auto c : _components)
            {
                c->execute();
            }
        }
};


#endif