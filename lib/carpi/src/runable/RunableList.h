#ifndef RUNABLELIST_H
#define RUNABLELIST_H

#include <vector>
#include "IRunable.h"

class RunableList : public IRunable {
    public:
        virtual void run() {
            for (auto &runable : _runables) {
                runable->run();
            }
        }

        /*
            Add a Runable to the List.
        */
        void addRunable(IRunable* runable) {
            _runables.push_back(runable);
        }

    protected:
        vector<IRunable*> _runables;
};

#endif // RUNABLELIST_H