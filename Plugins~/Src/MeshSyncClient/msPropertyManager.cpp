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

	void PropertyManager::updateFromServer(std::vector<PropertyInfo> properties, std::vector<CurvePtr> curves)
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		for (auto& prop : properties) {
			m_receivedProperties.push_back(prop);
		}

		for (auto& curve : curves) {
			m_receivedCurves.push_back(curve);
		}
	}

	std::vector<PropertyInfo> PropertyManager::getReceivedProperties()
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		return m_receivedProperties;
	}

	std::vector<CurvePtr> PropertyManager::getReceivedCurves() {
		std::unique_lock<std::mutex> lock(m_mutex);

		return m_receivedCurves;
	}

	void PropertyManager::clearReceivedProperties() {
		std::unique_lock<std::mutex> lock(m_mutex);

		m_receivedProperties.clear();
		m_receivedCurves.clear();
	}
}