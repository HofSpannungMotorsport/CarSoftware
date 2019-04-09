#ifndef IRUNABLE_H
#define IRUNABLE_H

class IRunable : private NonCopyable<IRunable> {
    public:
        virtual void run() = 0;
        virtual void run(Timer &timer) {
            timer.reset();
            timer.start();
            run();
            timer.stop();
        }
};

#endif // IRUNABLE_H