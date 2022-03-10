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

	void PropertyManager::updateFromServer(std::vector<PropertyInfo> properties)
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		for (auto prop : properties) {
			m_receivedProperties.push_back(prop);
		}
	}

	std::vector<PropertyInfo> PropertyManager::getReceivedProperties()
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		return m_receivedProperties;
	}

	void PropertyManager::clearReceivedProperties() {
		std::unique_lock<std::mutex> lock(m_mutex);

		m_receivedProperties.clear();
	}
}