#include "pch.h"

#include "MeshSyncClient/AsyncTasksController.h"

namespace MeshSyncClient {

void AsyncTasksController::Wait() {
    for (std::vector<std::future<void>>::value_type& t : m_tasks)
        t.wait();
    m_tasks.clear();
}



} //end namespace MeshSyncClient

