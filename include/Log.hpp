#ifndef _LOG_HPP_
#define _LOG_HPP_

#include <iostream>

class Log {
private:
    static std::ostream* sOstream;

public:

    static void setStream(std::ostream& ostr){
        sOstream = &ostr;
    }

    static std::ostream& E(const std::string &tag) {
        return (*sOstream) << "[Error] " << tag << ": ";
    }

    static std::ostream& W(const std::string &tag) {
        return (*sOstream) << "[Warning] " << tag << ": ";
    }

    static std::ostream& V(const std::string &tag) {
        return (*sOstream) << "[Verbose] " << tag << ": ";
    }

    static std::ostream& D(const std::string &tag) {
        return (*sOstream) << "[Debug] " << tag << ": ";
    }
};

#endif
