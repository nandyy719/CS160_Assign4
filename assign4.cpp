#include "lir.h"
#include "nlohmann_json.h"


int main(int argc, char* argv[]) {
    // Load JSON from a file
    std::ifstream file(argv[1]);
    json j;
    file >> j;

    json& glob = j["globals"];
    json& structs_json = j["structs"];
    json& externs_json = j["externs"];
    json& functions_json = j["functions"];

    Globals globals = parseGlobals(glob);
    Structs structs = parseStructs(structs_json);
    Externs externs = parseExterns(externs_json);
    Functions functions = parseFunctions(functions_json);

    functions.printFunctions();
   

    return 0;
}
