#include "Router.hpp"

#include <Utils.hpp>

const HandleFunc Router::NotFoundHandler = HANDLE_FUNC(){
    Log::E("Router") << "Path not found: " << RequestPathStr(request) << std::endl;
    //TODO
};

#define TRIM_PATH_SEP(path) \
    utils::TrimString((path), PATH_SEPARATOR)

Router& Router::Path(std::string path, HandleFunc func){
    /*
     * If you write /a/b
     * Property tree would split it into "", "a", "b"
     * Three identifiers instead of two("a" and "b") as we thought
     * So if you concate another sub-router like /c to /a/b
     * It would interpret the result identifiers as "", "a", "b", "", "c"
     * Instead of "a", "b", "c". Thus, /a/b/c path would cause path not found
     * The solution is simple: trim all the prefixing or suffixing '/'s in each path.
     */
    TRIM_PATH_SEP(path);
    callback_tree.put(GetPath(path), func);
    return *this;
}

Router& Router::Path(std::string path, const Router &sub_router){
    TRIM_PATH_SEP(path);
    callback_tree.put_child(GetPath(path), sub_router.callback_tree);
    return *this;
}

void Router::Process(const Request &request, const ResponseWriter &response_writer){
    std::string req_path = request.path()->str();
    TRIM_PATH_SEP(req_path);
    HandleFunc callback = callback_tree.get(GetPath(req_path), NotFoundHandler);
    callback(request, response_writer);
}

