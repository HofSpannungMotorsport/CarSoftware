#ifndef IPROGRAM_H
#define IPROGRAM_H

class IProgram : private NonCopyable<IProgram> {
    public:
        virtual void run() = 0;
};

#endif // IPROGRAM_H