#ifndef ISERVICE_H
#define ISERVICE_H

class IService : private NonCopyable<IService> {
    public:
        virtual void run() = 0;
};

#endif // ISERVICE_H