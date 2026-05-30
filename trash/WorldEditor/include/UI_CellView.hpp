#pragma once

namespace BunkerProtocol {
    class UICellView {
    public:
        void Draw(std::vector<MapObject>& objects, int& selectedIdx) {
            // Список: [Index] [ID] [Position]
            // При клике на строку - selectedIdx обновляется в EditorCore
        }
    };
}
