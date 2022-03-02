#include "MeshSyncClient/msPropertyManager.h"
#include "pch.h"
#include "MeshSync/SceneGraph/msPropertyInfo.h"

using namespace std;

namespace ms {
	vector<PropertyInfoPtr> ms::PropertyManager::getAllProperties()
	{
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
		auto rec = Record();
		rec.propertyInfo = propertyInfo;
		m_records.push_back(rec);
	}

	void PropertyManager::clear()
	{
		m_records.clear();
	}
}