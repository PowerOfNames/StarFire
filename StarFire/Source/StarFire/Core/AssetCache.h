#pragma once
#include "Aurora/Core/Core.h"
#include "Aurora/Renderer/Handles.h"
#include "Aurora/Core/StringKeyMap.h"

#include <string_view>

namespace Aurora {
	
	template<typename AssetHandleType, typename AssetType>
	class AssetCache
	{
	public:
		template<typename LoaderFunc>
		AssetHandleType Load(std::string_view key, LoaderFunc loader)
		{
			if (auto it = m_AssetHandles.find(key); it != m_AssetHandles.end())
				return it->second;

			Ref<AssetType> asset = loader();
			if (!asset)
			{
				//Fallback handle with ID = 0 -> telling renderer to use default
				using Tag = typename GetAssetTag<AssetHandleType>::Type;
				return AssetTraits<Tag>::GetFallback();
			}
				
			AssetHandleType handle = asset->CreateHandle();
			m_AssetHandles[std::string(key)] = handle;
			m_Assets[handle] = asset;
			return handle;
		}

	private:
		Containers::StringKeyMap<AssetHandleType> m_AssetHandles;
		//TODO: Replace with specialized asset allocator
		std::unordered_map<AssetHandleType, Ref<AssetType>> m_Assets;
	};
}
