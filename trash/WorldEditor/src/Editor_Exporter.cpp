#include "../include/Editor_Core.hpp"
#include <fstream>

void ExportWorld(const std::string& name, BunkerProtocol::EditorCore& core) {
    std::string path = "world/" + name + ".wld";
    std::ofstream file(path, std::ios::binary);

    file.write("BWLD", 4); // Заголовок
    
    auto& objects = core.GetScene();
    uint32_t count = objects.size();
    file.write(reinterpret_cast<char*>(&count), sizeof(count));
    file.write(reinterpret_cast<char*>(objects.data()), sizeof(BunkerProtocol::MapObject) * count);
    
    file.close();
}
