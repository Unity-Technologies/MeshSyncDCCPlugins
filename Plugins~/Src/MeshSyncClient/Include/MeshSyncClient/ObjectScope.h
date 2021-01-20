#pragma once

namespace MeshSyncClient {

enum class ObjectScope : int {
    None = -1,
    All,
    Selected,
    Updated,
};

} // namespace MeshSyncClient
