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
			ret.push_back(r.propertyInfo);
		}

		return ret;
	}

	void PropertyManager::add(PropertyInfoPtr propertyInfo)
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		// Make sure the same property doesn't get added twice when objects and their parents are iterated:
		for (size_t i = 0; i < m_records.size(); ++i) {
			if (m_records[i].propertyInfo->matches(propertyInfo)) {
				m_records[i].propertyInfo = propertyInfo;
				return;
			}
		}

		auto rec = Record();
		rec.propertyInfo = propertyInfo;
		m_records.push_back(rec);
	}

	void PropertyManager::clear()
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		m_records.clear();
	}

	void PropertyManager::updateFromServer(std::vector<PropertyInfo> properties, std::vector<EntityPtr> entities)
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		m_receivedProperties.insert(m_receivedProperties.end(), properties.begin(), properties.end());
		m_receivedEntities.insert(m_receivedEntities.end(), entities.begin(), entities.end());
	}

	std::vector<PropertyInfo> PropertyManager::getReceivedProperties()
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		return m_receivedProperties;
	}

	std::vector<EntityPtr> PropertyManager::getReceivedEntities() {
		std::unique_lock<std::mutex> lock(m_mutex);

		return m_receivedEntities;
	}

	void PropertyManager::clearReceivedData() {
		std::unique_lock<std::mutex> lock(m_mutex);

		m_receivedProperties.clear();
		m_receivedEntities.clear();
	}
}