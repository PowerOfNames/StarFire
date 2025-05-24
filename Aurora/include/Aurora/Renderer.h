#pragma once



namespace Aurora {


	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		void Init(const char* message);
		void Shutdown();


	private:
		const char* m_Message = nullptr;
	};

}