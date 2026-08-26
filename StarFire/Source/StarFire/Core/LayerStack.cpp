#include "sfpch.h"
#include "StarFire/Core/LayerStack.h"

namespace StarFire {


	LayerStack::LayerStack() {}

	LayerStack::~LayerStack()
	{
		PROFILE_FUNCTION;

		Clear();
	}

	void LayerStack::Clear()
	{
		PROFILE_FUNCTION;

		//With this, we clear the layers in LIFO order, starting with the overlays
		while(!m_Layers.empty())
		{
			Layer* layer = m_Layers.back();
			PopLayer(layer);
			delete layer;
		}
	}

	void LayerStack::PushLayer(Layer* layer)
	{
		PROFILE_FUNCTION;

		m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
		layer->OnAttach();
		m_LayerInsertIndex++;
	}


	void LayerStack::PopLayer(Layer* layer)
	{
		PROFILE_FUNCTION;

		auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
		if (it != m_Layers.end())
		{
			layer->OnDetach();
			m_Layers.erase(it);
			m_LayerInsertIndex--;
		}
	}


	void LayerStack::PushOverlay(Layer* overlay)
	{
		PROFILE_FUNCTION;

		m_Layers.emplace_back(overlay);
		overlay->OnAttach();
	}


	void LayerStack::PopOverlay(Layer* overlay)
	{
		PROFILE_FUNCTION;

		auto it = std::find(m_Layers.begin(), m_Layers.end(), overlay);
		if (it != m_Layers.end())
		{
			overlay->OnDetach();
			m_Layers.erase(it);
		}
	}

}
