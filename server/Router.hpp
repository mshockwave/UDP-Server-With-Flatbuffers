#ifndef _SERVER_ROUTER_HPP_
#define _SERVER_ROUTER_HPP_

#include <string>
#include <functional>

#include <Log.hpp>
#include "Types.hpp"

#include <schemas/packet_generated.h>
#include <boost/property_tree/ptree.hpp>

typedef fbs::RequestPacket Request;
#define RequestPathStr(req) \
    (req).path()->str()

typedef std::function<void(const Request&,const ResponseWriter&)> HandleFunc;
#define HANDLE_FUNC() \
    [](const Request& request, const ResponseWriter& response_writer)->void

class Router {
    
public:
    
    static const HandleFunc NotFoundHandler;
    
    Router(){/*Default construtor*/}
    Router(const Router& that){
        //Copy constructor
        callback_tree = that.callback_tree;
    }
    
    void Process(const Request&, const ResponseWriter&);
    
    //Handler function
    Router& Path(std::string, HandleFunc);
    
    //Sub-router
    Router& Path(std::string, const Router&);
    
private:
    
    boost::property_tree::basic_ptree<std::string, HandleFunc> callback_tree;
    
    inline boost::property_tree::path get_path(const std::string &path){
        return boost::property_tree::path(path, PATH_SEPARATOR);
    }
};

#endif