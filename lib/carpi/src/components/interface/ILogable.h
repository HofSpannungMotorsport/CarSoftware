#ifndef ILOGABLE_H
#define ILOGABLE_H

#include <string>
using namespace std;

typedef uint8_t status_t;
typedef uint8_t sd_log_id_t;

class ILogable : private NonCopyable<ILogable> {
    public:
        virtual void setStatus(status_t status) = 0;
        virtual status_t getStatus() = 0;

        virtual sd_log_id_t getLogValueCount() {
            return 0;
        }

        virtual void getLogValue(string &logValue, sd_log_id_t logId) {
            
        }
};

#endif // ILOGABLE_H