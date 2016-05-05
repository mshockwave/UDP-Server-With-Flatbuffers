#include "Router.hpp"

#include <Utils.hpp>

const HandleFunc Router::NotFoundHandler = HANDLE_FUNC(){
    Log::E("Router") << "Path not found: " << RequestPathStr(request) << std::endl;
    //TODO
};

#define TRIM_PATH_SEP(path) \
    utils::TrimString((path), PATH_SEPARATOR)

Router& Router::Path(std::string path, HandleFunc func){
    /*Remove prefix separator to avoid path mismatching*/
    TRIM_PATH_SEP(path);
    callback_tree.put(get_path(path), func);
    return *this;
}

Router& Router::Path(std::string path, const Router &sub_router){
    TRIM_PATH_SEP(path);
    callback_tree.put_child(get_path(path), sub_router.callback_tree);
    return *this;
}

void Router::Process(const Request &request, const ResponseWriter &response_writer){
    std::string req_path = request.path()->str();
    TRIM_PATH_SEP(req_path);
    HandleFunc callback = callback_tree.get(get_path(req_path), NotFoundHandler);
    callback(request, response_writer);
}

