#ifndef ILOGABLE_H
#define ILOGABLE_H

#include <string>
using namespace std;

typedef uint8_t sd_log_id_t;

class ILogable : private NonCopyable<ILogable> {
    public:
        virtual sd_log_id_t getLogValueCount() {
            return 0;
        }

        virtual void getLogValue(string &logValue, sd_log_id_t logId) {
            
        }
};

#endif // ILOGABLE_H