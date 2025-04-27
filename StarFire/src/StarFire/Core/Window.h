#pragma once
#include "StarFire/Events/Event.h"

#include <memory>


namespace StarFire {

	enum class CursorState
	{
		NONE = 0,
		NORMAL,			// normal behaviour
		DISABLED,		// used for camera movement only
		HIDDEN,			// hidden, but works normally
		CAPTURED		// visible, but bound to the window content area until focus lost
	};

	struct WindowSpecification
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;

		uint32_t PositionX;
		uint32_t PositionY;

		bool Fullscreen;
		bool VSync;

		WindowSpecification(const std::string& title = "StarFire Engine", bool fullScreen = false, bool vSync = false, uint32_t width = 1600, uint32_t height = 900, uint32_t posX = 100, uint32_t posY = 50)
			: Title(title), Fullscreen(fullScreen), VSync(vSync), Width(width), Height(height), PositionX(posX), PositionY(posY)
		{}


		CursorState MouseCursorState = CursorState::NORMAL;
	};

	class Window
	{
	public:
		using EventCallbackFN = std::function<void(Event&)>;
		virtual ~Window() = default;

		virtual void Init() = 0;
		virtual void Close() = 0;		

		virtual void OnUpdate() = 0;
		virtual void PollEvents() = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual const WindowSpecification& GetSpecification() const = 0;

		virtual void SetEventCallback(const EventCallbackFN& eventCallback) = 0;

		//Window Attributes
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;
		virtual void SetFullscreen(bool enabled) = 0;
		virtual bool IsFullscreen() const = 0;

		virtual void SetCursorState(CursorState state) = 0;
		virtual CursorState GetCursorState() const = 0;

		virtual void* GetNativeWindow() const = 0;


		static std::unique_ptr<Window> Create(const WindowSpecification& specs = WindowSpecification());
	};
}
