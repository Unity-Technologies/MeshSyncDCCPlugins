#include "MeshSyncClient/msPropertyManager.h"
#include "pch.h"
#include "MeshSync/SceneGraph/msPropertyInfo.h"

using namespace std;

namespace ms {
	vector<PropertyInfoPtr> ms::PropertyManager::getAllProperties()
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		vector<PropertyInfoPtr> ret;

		for (auto& r : m_records) {
			if (r.dirty) {
				ret.push_back(r.propertyInfo);
			}
		}

		return ret;
	}

	void PropertyManager::add(PropertyInfoPtr propertyInfo)
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		auto rec = Record();
		rec.propertyInfo = propertyInfo;
		m_records.push_back(rec);
	}

	void PropertyManager::clear()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_records.clear();
	}
}